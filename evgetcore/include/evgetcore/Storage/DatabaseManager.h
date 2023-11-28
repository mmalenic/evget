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

#include "async/coroutine/Scheduler.h"

namespace EvgetCore::Storage {

namespace asio = boost::asio;
namespace lockfree = boost::lockfree;

class DatabaseManager : public Store {
public:
    /**
     * \brief Construct a database manager
     * \param nEvents the number of events to hold before inserting.
     */
    DatabaseManager(Async::Scheduler& scheduler, std::vector<std::reference_wrapper<Store>> storeIn, size_t nEvents);

    asio::awaitable<Result<void>> store(Event::Data event) override;

private:
    std::reference_wrapper<Async::Scheduler> scheduler;
    std::vector<std::reference_wrapper<Store>> storeIn;
    size_t nEvents;
    // lockfree::queue<Event::Data> events;

    asio::awaitable<Result<void>> storeWith(Event::Data event);
};

}

#endif //DATABASEMANAGER_H
