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

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "Store.h"
#include <boost/lockfree/queue.hpp>
#include <spdlog/spdlog.h>
#include <boost/asio.hpp>

namespace EvgetCore::Storage {

namespace asio = boost::asio;
namespace lockfree = boost::lockfree;

template <asio::execution::executor E>
class DatabaseManager : public Store {
public:
    /**
     * \brief Construct a database manager
     * \param nEvents the number of events to hold before inserting.
     */
    DatabaseManager(E& context, Store& storeIn, size_t nEvents);

    asio::awaitable<Result<void>> store(Event::Data event) override;

private:
    std::reference_wrapper<E> context;
    std::reference_wrapper<Store> storeIn;
    size_t nEvents;
    // lockfree::queue<Event::Data> events;

    asio::awaitable<Result<void>> storeWith(Event::Data event);
};

template <asio::execution::executor E>
DatabaseManager<E>::DatabaseManager(E& context, Store& storeIn, size_t nEvents) :
context{context}, storeIn{storeIn}, nEvents{nEvents} {
}

template <asio::execution::executor E>
asio::awaitable<Result<void>> DatabaseManager<E>::store(Event::Data event) {
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

template <asio::execution::executor E>
asio::awaitable<Result<void>> DatabaseManager<E>::storeWith(Event::Data event) {
    co_return co_await storeIn.get().store(std::move(event));
}

}

#endif //DATABASEMANAGER_H
