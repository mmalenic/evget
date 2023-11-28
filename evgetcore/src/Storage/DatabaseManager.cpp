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

EvgetCore::Storage::DatabaseManager::DatabaseManager(Async::Scheduler& scheduler, Store& storeIn, size_t nEvents) :
scheduler{scheduler}, storeIn{storeIn}, nEvents{nEvents} {
}

EvgetCore::Storage::asio::awaitable<EvgetCore::Storage::Result<void>> EvgetCore::Storage::DatabaseManager::store(Event::Data event) {
    // auto success = events.bounded_push(std::move(event));

    Result<void> outResult = {};
    // if (!success) {
    //     spdlog::info("reached end of queue, storing events.");
    //
    //     Event::Data out{};
    //     Event::Data value{};
    //     while (events.pop(value)) {
    //         out.mergeWith(std::move(value));
    //     }
    //
    //     asio::co_spawn(context, [this, out]() {
    //         return this->storeWith(std::move(out));
    //     }, [&outResult](std::exception_ptr e, Result<void> result) {
    //         if (e != nullptr) {
    //           try {
    //             std::rethrow_exception(e);
    //           } catch (std::exception &e) {
    //               auto what = e.what();
    //               spdlog::error("error when storing in database manager: {}", what);
    //               outResult = Err{{.errorType = ErrorType::DatabaseManager, .message = what}};
    //           }
    //         }
    //         if (!result.has_value()) {
    //             outResult = Err{result.error()};
    //         }
    //     });
    // }

    co_return outResult;
}

EvgetCore::Storage::asio::awaitable<EvgetCore::Storage::Result<void>> EvgetCore::Storage::DatabaseManager::storeWith(Event::Data event) {
    co_return co_await storeIn.get().store(std::move(event));
}