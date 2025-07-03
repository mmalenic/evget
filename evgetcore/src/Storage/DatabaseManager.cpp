// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "evgetcore/Storage/DatabaseManager.h"
#include "evgetcore/Storage/Store.h"

#include <chrono>
#include <cstddef>
#include <expected>
#include <format>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include "evgetcore/Error.h"
#include "evgetcore/Event/Data.h"
#include "evgetcore/async/scheduler/Scheduler.h"

EvgetCore::Storage::DatabaseManager::DatabaseManager(
    std::shared_ptr<Scheduler> scheduler,
    std::vector<std::shared_ptr<Store>> storeIn,
    std::size_t nEvents,
    std::chrono::seconds storeAfter
)
    : scheduler{std::move(scheduler)}, storeIn{std::move(storeIn)}, nEvents{nEvents}, storeAfter{storeAfter} {
    spawn_store_after();
}

void EvgetCore::Storage::DatabaseManager::spawn_store_data(std::optional<std::vector<Event::Data>> inner, std::vector<std::shared_ptr<Store>> store_in, Scheduler& scheduler) {
    if (inner.has_value()) {
        spdlog::info(fmt::format("reached threshold, storing {} events", inner->size()));

        Event::Data out{};
        for (auto data : *std::move(inner)) {
            out.mergeWith(std::move(data));
        }

        scheduler.spawn<Result<void>>(
            store_coroutine(out, std::move(store_in)),
            [&scheduler](Result<void> result) {
                resultHandler(std::move(result), scheduler);
            }
        );
    }
}

EvgetCore::Result<void> EvgetCore::Storage::DatabaseManager::store(Event::Data event) {
    data->push_back(std::move(event));
    Result<void> outResult = {};

    auto inner = data->into_inner_at(nEvents);
    spawn_store_data(inner, storeIn, *scheduler);

    return outResult;
}

void EvgetCore::Storage::DatabaseManager::add_store(std::unique_ptr<Store> store) {
    this->storeIn.emplace_back(std::move(store));
}

boost::asio::awaitable<EvgetCore::Result<void>> EvgetCore::Storage::DatabaseManager::store_coroutine(Event::Data data, std::vector<std::shared_ptr<Store>> store_in) {
    for (const auto& store : store_in) {
        auto result = store->store(data);

        if (!result.has_value()) {
            co_return result;
        }
    }

    co_return Result<void>{};
}

boost::asio::awaitable<std::expected<void, EvgetCore::Error<EvgetCore::ErrorType>>> EvgetCore::Storage::DatabaseManager::store_after_coroutine(std::shared_ptr<Scheduler> scheduler, std::shared_ptr<LockingVector<Event::Data>> data, std::vector<std::shared_ptr<Store>> store_in, std::chrono::seconds storeAfter) {
    auto interval = Interval{storeAfter};
    while (!co_await scheduler->isStopped()) {
        auto result = co_await interval.tick();

        spdlog::debug(
            std::format("timer threshold of {} seconds reached", interval.period().count())
        );

        if (!result.has_value()) {
            co_return Err{Error{.errorType = ErrorType::DatabaseManager, .message = result.error().message}};
        }

        auto data_inner = data->into_inner();
        spawn_store_data(data_inner, store_in, *scheduler);
    }

    co_return Result<void>{};
}

void EvgetCore::Storage::DatabaseManager::spawn_store_after() const {
    scheduler->spawn<Result<void>>(
        store_after_coroutine(scheduler, data, storeIn, storeAfter),
        [this](Result<void> result) {
            resultHandler(std::move(result), *this->scheduler);
        }
    );
}

void EvgetCore::Storage::DatabaseManager::resultHandler(Result<void> result, Scheduler &scheduler) {
    if (!result.has_value()) {
        spdlog::error("Error storing events: {}", result.error().message);
        scheduler.stop();
    }
}
