#include "evget/storage/database_manager.h"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>

#include "common/store.h"
#include "evget/async/scheduler/scheduler.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace {

using test::StoreErrorMock;
using test::StoreMock;

} // namespace

TEST(DatabaseManagerTest, EventsBelowThresholdNotFlushed) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {store}, 3, std::chrono::seconds{60}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    scheduler->Stop();
    scheduler->Join();

    ASSERT_TRUE(store->Events().empty());
}

TEST(DatabaseManagerTest, EventsAtThresholdFlushed) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {store}, 2, std::chrono::seconds{60}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    store->WaitForEvents(1);
    scheduler->Stop();
    scheduler->Join();

    auto events = store->Events();
    ASSERT_EQ(events.size(), 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    ASSERT_EQ(events[0].Entries().size(), 2);
}

TEST(DatabaseManagerTest, MultipleEvents) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {store}, 2, std::chrono::seconds{60}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    store->WaitForEvents(1);
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    store->WaitForEvents(2);
    scheduler->Stop();
    scheduler->Join();

    auto events = store->Events();
    ASSERT_EQ(events.size(), 2);
}

TEST(DatabaseManagerTest, MultipleStoresReceiveEvents) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store_one = std::make_shared<StoreMock>();
    auto store_two = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {store_one, store_two}, 1, std::chrono::seconds{60}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    store_one->WaitForEvents(1);
    store_two->WaitForEvents(1);
    scheduler->Stop();
    scheduler->Join();

    ASSERT_EQ(store_one->Events().size(), 1);
    ASSERT_EQ(store_two->Events().size(), 1);
}

TEST(DatabaseManagerTest, AddStoreReceivesEvents) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto shared_store = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {shared_store}, 1, std::chrono::seconds{60}};
    manager.AddStore(std::make_unique<StoreMock>());

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    shared_store->WaitForEvents(1);
    scheduler->Stop();
    scheduler->Join();

    ASSERT_EQ(shared_store->Events().size(), 1);
}

TEST(DatabaseManagerTest, TimerFlushesBelowThreshold) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {store}, 100, std::chrono::seconds{1}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    store->WaitForEvents(1);
    scheduler->Stop();
    scheduler->Join();

    ASSERT_EQ(store->Events().size(), 1);
}

TEST(DatabaseManagerTest, StoreErrorStopsScheduler) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreErrorMock>();

    evget::DatabaseManager manager{scheduler, {store}, 1, std::chrono::seconds{60}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    // The error handler calls scheduler->Stop()
    scheduler->Join();
}

TEST(DatabaseManagerTest, StoreEventReturnsSuccess) {
    auto scheduler = std::make_shared<evget::Scheduler>();

    evget::DatabaseManager manager{scheduler, {}, 5, std::chrono::seconds{60}};

    auto result = manager.StoreEvent(StoreMock::MakeData());
    ASSERT_TRUE(result.has_value());

    scheduler->Stop();
    scheduler->Join();
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
