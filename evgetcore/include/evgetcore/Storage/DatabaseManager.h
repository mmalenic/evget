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

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include "Store.h"
#include "async/container/LockingVector.h"
#include "async/scheduler/Interval.h"
#include "async/scheduler/Scheduler.h"

namespace EvgetCore::Storage {

namespace asio = boost::asio;

class DatabaseManager : public Store {
public:
    /**
     * \brief Construct a database manager
     * \param scheduler scheduler to use.
     * \param storeIn stores to store events in.
     * \param nEvents the number of events to hold before inserting.
     * \param storeAfter store events after this time event if nEvents is not reached.
     */
    DatabaseManager(Async::Scheduler& scheduler, std::vector<std::reference_wrapper<Store>> storeIn, size_t nEvents = 100, std::chrono::seconds storeAfter = std::chrono::seconds{60});

    Result<void> store(Event::Data event) override;

private:
    void storeEventsTask(std::optional<std::vector<Event::Data>> events);
    void storeAfterTask();

    std::reference_wrapper<Async::Scheduler> scheduler;
    std::vector<std::reference_wrapper<Store>> storeIn;
    size_t nEvents;
    Async::Interval storeAfterInterval;
    Async::LockingVector<Event::Data> data;

    Result<void> storeWith(Event::Data event);
    void resultHandler(Result<void> result);
};

}

#endif //DATABASEMANAGER_H
