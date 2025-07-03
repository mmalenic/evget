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

#include <cstddef>
#include <vector>

#include "Store.h"
#include "evgetcore/async/container/LockingVector.h"
#include "evgetcore/async/scheduler/Interval.h"
#include "evgetcore/async/scheduler/Scheduler.h"

namespace EvgetCore::Storage {

class DatabaseManager : public Store {
public:
    /**
     * \brief Construct a database manager
     * \param scheduler scheduler to use.
     * \param storeIn stores to store events in.
     * \param nEvents the number of events to hold before inserting.
     * \param storeAfter store events after this time event if nEvents is not reached.
     */
    explicit DatabaseManager(
        std::shared_ptr<EvgetCore::Scheduler> scheduler,
        std::vector<std::shared_ptr<Store>> storeIn,
        std::size_t nEvents,
        std::chrono::seconds storeAfter
    );

    Result<void> store(Event::Data event) override;

    void add_store(std::unique_ptr<Store> store);

private:
    static void spawn_store_data(std::optional<std::vector<Event::Data>> inner, std::vector<std::shared_ptr<Store>> store_in, Scheduler& scheduler);
    static asio::awaitable<Result<void>> store_coroutine(Event::Data data, std::vector<std::shared_ptr<Store>> store_in);
    static asio::awaitable<Result<void>> store_after_coroutine(std::shared_ptr<Scheduler> scheduler, std::shared_ptr<LockingVector<Event::Data>> data, std::vector<std::shared_ptr<Store>> store_in, std::chrono::seconds storeAfter);
    static void resultHandler(Result<void> result, Scheduler& scheduler);

    void spawn_store_after() const;

    std::shared_ptr<EvgetCore::Scheduler> scheduler;
    std::vector<std::shared_ptr<Store>> storeIn;
    size_t nEvents;
    std::chrono::seconds storeAfter;
    std::shared_ptr<EvgetCore::LockingVector<Event::Data>> data;
};

}  // namespace EvgetCore::Storage

#endif  // DATABASEMANAGER_H
