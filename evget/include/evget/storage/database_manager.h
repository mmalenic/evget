#ifndef EVGET_STORAGE_DATABASE_MANAGER_H
#define EVGET_STORAGE_DATABASE_MANAGER_H

#include <boost/asio/awaitable.hpp>

#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "evget/async/container/locking_vector.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/storage/store.h"

namespace evget {

class DatabaseManager : public Store {
public:
    /**
     * \brief Construct a database manager.
     * \param scheduler scheduler to use
     * \param store_in stores to store events in
     * \param n_events the number of events to hold before inserting
     * \param store_after store events after this time event if `n_events` is not reached
     */
    DatabaseManager(
        std::shared_ptr<Scheduler> scheduler,
        std::vector<std::shared_ptr<Store>> store_in,
        std::size_t n_events,
        std::chrono::seconds store_after
    );

    Result<void> StoreEvent(Data event) override;

    /**
     * \brief Add a store to the database manager.
     * \param store unique pointer to the store to add
     */
    void AddStore(std::unique_ptr<Store> store);

private:
    static void SpawnStoreData(
        std::optional<std::vector<Data>> inner,
        std::vector<std::shared_ptr<Store>> store_in,
        Scheduler& scheduler
    );
    static boost::asio::awaitable<Result<void>> StoreCoroutine(Data data, std::vector<std::shared_ptr<Store>> store_in);
    static boost::asio::awaitable<Result<void>> StoreAfterCoroutine(
        std::shared_ptr<Scheduler> scheduler,
        std::shared_ptr<LockingVector<Data>> data,
        std::vector<std::shared_ptr<Store>> store_in,
        std::chrono::seconds store_after
    );
    static void ResultHandler(Result<void> result, Scheduler& scheduler);

    void SpawnStoreAfter() const;

    std::shared_ptr<Scheduler> scheduler_;
    std::vector<std::shared_ptr<Store>> store_in_;
    size_t n_events_{};
    std::chrono::seconds store_after_{};
    std::shared_ptr<LockingVector<Data>> data_;
};

} // namespace evget

#endif
