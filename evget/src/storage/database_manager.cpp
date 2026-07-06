#include "evget/storage/database_manager.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp> // NOLINT(misc-include-cleaner)
#include <boost/asio/strand.hpp>
#include <boost/asio/use_future.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstddef>
#include <expected>
#include <format>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "evget/async/container/locking_vector.h"
#include "evget/async/scheduler/interval.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/storage/store.h"

evget::DatabaseManager::DatabaseManager(
    std::shared_ptr<Scheduler> scheduler,
    std::vector<std::shared_ptr<Store>> store_in,
    std::size_t n_events,
    std::chrono::seconds store_after
)
    : scheduler_{std::move(scheduler)},
      strand_{boost::asio::make_strand(scheduler_->Executor())},
      store_in_{std::make_shared<StoresHolder>()},
      n_events_{n_events},
      store_after_{store_after} {
    store_in_->stores = std::move(store_in);
    SpawnStoreAfter();
}

std::vector<std::shared_ptr<evget::Store>> evget::DatabaseManager::Snapshot(StoresHolder& holder) {
    const std::scoped_lock lock{holder.lock};
    return holder.stores;
}

void evget::DatabaseManager::SpawnStoreData(
    std::optional<std::vector<Data>> inner,
    std::vector<std::shared_ptr<Store>> store_in,
    Scheduler& scheduler,
    boost::asio::strand<boost::asio::any_io_executor> strand
) {
    if (inner.has_value()) {
        spdlog::info(std::format("reached threshold, storing {} events", inner->size()));

        Data out{};
        for (auto&& data : *std::move(inner)) {
            out.MergeWith(std::move(data));
        }

        scheduler.Spawn(std::move(strand), StoreCoroutine(out, std::move(store_in)), [&scheduler](Result<void> result) {
            ResultHandler(std::move(result), scheduler);
        });
    }
}

evget::Result<void> evget::DatabaseManager::StoreEvent(Data events) {
    data_->PushBack(std::move(events));

    auto inner = data_->IntoInnerAt(n_events_);
    if (inner.has_value()) {
        SpawnStoreData(std::move(inner), Snapshot(*store_in_), *scheduler_, strand_);
    }

    return {};
}

void evget::DatabaseManager::AddStore(std::unique_ptr<Store> store) const {
    const std::scoped_lock lock{store_in_->lock};
    store_in_->stores.emplace_back(std::move(store));
}

void evget::DatabaseManager::Flush() {
    auto inner = data_->IntoInner();
    if (!inner.has_value() || inner->empty()) {
        return;
    }

    spdlog::info(std::format("flushing {} buffered events", inner->size()));

    Data out{};
    for (auto&& data : *std::move(inner)) {
        out.MergeWith(std::move(data));
    }

    // Runs on the signal thread while the pool is alive, so blocking on the future cannot deadlock.
    // NOLINTNEXTLINE(misc-include-cleaner)
    auto future = boost::asio::co_spawn(strand_, StoreCoroutine(out, Snapshot(*store_in_)), boost::asio::use_future);
    auto result = future.get();
    if (!result.has_value()) {
        spdlog::error("Error flushing events: {}", result.error().message);
    }
}

boost::asio::awaitable<evget::Result<void>>
evget::DatabaseManager::StoreCoroutine(Data data, std::vector<std::shared_ptr<Store>> store_in) {
    for (const auto& store : store_in) {
        auto result = store->StoreEvent(data);

        if (!result.has_value()) {
            co_return result;
        }
    }

    co_return Result<void>{};
}

boost::asio::awaitable<std::expected<void, evget::Error<evget::ErrorType>>> evget::DatabaseManager::StoreAfterCoroutine(
    std::weak_ptr<Scheduler> scheduler_weak,
    std::shared_ptr<LockingVector<Data>> data,
    std::shared_ptr<StoresHolder> store_in,
    std::chrono::seconds store_after,
    boost::asio::strand<boost::asio::any_io_executor> strand
) {
    auto store_interval = Interval{store_after};
    // Use a weak_ptr here to break cycle between scheduler and database manager, this must
    // live in a block that ends before the next co_await to avoid re-introducing the cycle.
    while (true) {
        {
            auto scheduler = scheduler_weak.lock();
            if (!scheduler || scheduler->IsStopped()) {
                break;
            }
        }

        auto result = co_await store_interval.Tick();

        spdlog::debug(std::format("timer threshold of {} seconds reached", store_interval.Period().count()));

        if (!result.has_value()) {
            co_return Err{Error{.error_type = ErrorType::kDatabaseManagerError, .message = result.error().message}};
        }

        {
            auto scheduler = scheduler_weak.lock();
            if (!scheduler) {
                break;
            }

            auto data_inner = data->IntoInner();
            SpawnStoreData(data_inner, Snapshot(*store_in), *scheduler, strand);
        }
    }

    co_return Result<void>{};
}

void evget::DatabaseManager::SpawnStoreAfter() const {
    std::weak_ptr<Scheduler> weak_scheduler = scheduler_;
    scheduler_->Spawn<Result<void>>(
        StoreAfterCoroutine(std::move(weak_scheduler), data_, store_in_, store_after_, strand_),
        [this](Result<void> result) { ResultHandler(std::move(result), *this->scheduler_); }
    );
}

void evget::DatabaseManager::ResultHandler(Result<void> result, Scheduler& scheduler) {
    if (!result.has_value()) {
        spdlog::error("Error storing events: {}", result.error().message);
        scheduler.Stop();
    }
}
