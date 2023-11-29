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

EvgetCore::Storage::DatabaseManager::DatabaseManager(Async::Scheduler& scheduler, std::vector<std::reference_wrapper<Store>> storeIn, size_t nEvents) :
scheduler{scheduler}, storeIn{std::move(storeIn)}, nEvents{nEvents}, data{} {
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::DatabaseManager::store(Event::Data event) {
    data.push_back(std::move(event));
    Result<void> outResult = {};

    auto intoInner = data.into_inner_at(nEvents);
    if (intoInner.has_value()) {
        spdlog::info("reached event number threshold, storing events.");

        Event::Data out{};
        for (auto data : *intoInner) {
            out.mergeWith(std::move(data));
        }

        scheduler.get().spawn<Result<void>>([this, out]() -> asio::awaitable<Result<void>> {
            co_return this->storeWith(out);
        }, [this](Result<void> result) {
            this->resultHandler(result);
        });
    }

    return outResult;
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::DatabaseManager::storeWith(Event::Data event) {
    for (auto store : storeIn) {
        auto result = store.get().store(event);

        if (!result.has_value()) {
            return result;
        }
    }

    return Result<void>{};
}

void EvgetCore::Storage::DatabaseManager::resultHandler(Result<void> result) {
    if (!result.has_value()) {
        spdlog::error("Error storing events: {}", result.error().message);
        scheduler.get().stop();
    }
}