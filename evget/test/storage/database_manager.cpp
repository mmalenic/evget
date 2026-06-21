#include "evget/storage/database_manager.h"

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <memory>

#include "common/database.h"
#include "common/store.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"
#include "evget/event/data.h"
#include "evget/storage/store.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace {

using test::ConcurrencyStore;
using test::StoreErrorMock;
using test::StoreForwarder;
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
    auto constructor_store = std::make_shared<StoreMock>();
    auto added_store = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {constructor_store}, 1, std::chrono::seconds{60}};
    manager.AddStore(std::make_unique<StoreForwarder>(added_store));

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    constructor_store->WaitForEvents(1);
    added_store->WaitForEvents(1);
    scheduler->Stop();
    scheduler->Join();

    ASSERT_EQ(constructor_store->Events().size(), 1);
    ASSERT_EQ(added_store->Events().size(), 1);
}

TEST(DatabaseManagerTest, AddStoreVisibleToTimerFlush) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto added_store = std::make_shared<StoreMock>();

    // High n_events ensures only the timer will flush.
    evget::DatabaseManager manager{scheduler, {}, 100, std::chrono::seconds{1}};
    manager.AddStore(std::make_unique<StoreForwarder>(added_store));

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    added_store->WaitForEvents(1);
    scheduler->Stop();
    scheduler->Join();

    ASSERT_EQ(added_store->Events().size(), 1);
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

TEST(DatabaseManagerTest, ConcurrentFlushesNeverOverlap) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto probe = std::make_shared<ConcurrencyStore>();

    evget::DatabaseManager manager{scheduler, {probe}, 2, std::chrono::seconds{60}};

    for (std::size_t i = 0; i < 40; ++i) {
        ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    }

    probe->WaitForEntries(40);
    scheduler->Stop();
    scheduler->Join();

    ASSERT_FALSE(probe->Concurrent());
    ASSERT_EQ(probe->TotalEntries(), 40);
}

TEST(DatabaseManagerTest, FlushDrainsBelowThresholdBuffer) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();

    evget::DatabaseManager manager{scheduler, {store}, 100, std::chrono::seconds{600}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    manager.Flush();

    auto events = store->Events();
    ASSERT_EQ(events.size(), 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    ASSERT_EQ(events[0].Entries().size(), 3);

    scheduler->Stop();
    scheduler->Join();
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
