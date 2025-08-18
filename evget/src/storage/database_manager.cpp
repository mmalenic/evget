#include "evget/storage/database_manager.h"

#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstddef>
#include <expected>
#include <format>
#include <memory>
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
    : scheduler_{std::move(scheduler)}, store_in_{std::move(store_in)}, n_events_{n_events}, store_after_{store_after} {
    SpawnStoreAfter();
}

void evget::DatabaseManager::SpawnStoreData(
    std::optional<std::vector<Data>> inner,
    std::vector<std::shared_ptr<Store>> store_in,
    Scheduler& scheduler
) {
    if (inner.has_value()) {
        spdlog::info(std::format("reached threshold, storing {} events", inner->size()));

        Data out{};
        for (auto data : *std::move(inner)) {
            out.MergeWith(std::move(data));
        }

        scheduler.Spawn<Result<void>>(StoreCoroutine(out, std::move(store_in)), [&scheduler](Result<void> result) {
            ResultHandler(std::move(result), scheduler);
        });
    }
}

evget::Result<void> evget::DatabaseManager::StoreEvent(Data events) {
    data_->PushBack(std::move(events));
    const Result<void> out_result = {};

    auto inner = data_->IntoInnerAt(n_events_);
    SpawnStoreData(inner, store_in_, *scheduler_);

    return out_result;
}

void evget::DatabaseManager::AddStore(std::unique_ptr<Store> store) {
    this->store_in_.emplace_back(std::move(store));
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
    std::shared_ptr<Scheduler> scheduler,
    std::shared_ptr<LockingVector<Data>> data,
    std::vector<std::shared_ptr<Store>> store_in,
    std::chrono::seconds store_after
) {
    auto store_interval = Interval{store_after};
    while (!co_await scheduler->IsStopped()) {
        auto result = co_await store_interval.Tick();

        spdlog::debug(std::format("timer threshold of {} seconds reached", store_interval.Period().count()));

        if (!result.has_value()) {
            co_return Err{Error{.error_type = ErrorType::kDatabaseManager, .message = result.error().message}};
        }

        auto data_inner = data->IntoInner();
        SpawnStoreData(data_inner, store_in, *scheduler);
    }

    co_return Result<void>{};
}

void evget::DatabaseManager::SpawnStoreAfter() const {
    scheduler_->Spawn<Result<void>>(
        StoreAfterCoroutine(scheduler_, data_, store_in_, store_after_),
        [this](Result<void> result) { ResultHandler(std::move(result), *this->scheduler_); }
    );
}

void evget::DatabaseManager::ResultHandler(Result<void> result, Scheduler& scheduler) {
    if (!result.has_value()) {
        spdlog::error("Error storing events: {}", result.error().message);
        scheduler.Stop();
    }
}
