#include "evget/shutdown.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "common/event_loop.h"
#include "common/log.h"
#include "common/store.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/event_loop.h"
#include "evget/storage/database_manager.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace {

using test::CapturedLog;
using test::QueuedNextEvent;
using test::RecordingListener;
using test::StoreMock;
using test::ThrowingNextEvent;

} // namespace

TEST(ShutdownTest, StopsSourceFlushAndScheduler) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    std::mutex mutex;
    std::vector<std::string> steps;

    evget::Shutdown shutdown{scheduler, [&mutex, &steps, &scheduler] {
                                 const std::scoped_lock lock{mutex};
                                 steps.emplace_back(scheduler->IsStopped() ? "flush after stop" : "flush");
                             }};
    shutdown.SetSource([&mutex, &steps] {
        const std::scoped_lock lock{mutex};
        steps.emplace_back("source");
    });

    shutdown.Request();
    scheduler->Join();

    ASSERT_EQ(steps, (std::vector<std::string>{"source", "flush"}));
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, RunConcurrently) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    std::atomic<int> sources{0};
    std::atomic<int> flushes{0};

    evget::Shutdown shutdown{scheduler, [&flushes] { flushes.fetch_add(1); }};
    shutdown.SetSource([&sources] { sources.fetch_add(1); });

    {
        const std::jthread first{[&shutdown] { shutdown.Request(); }};
        const std::jthread second{[&shutdown] { shutdown.Request(); }};
    }
    scheduler->Join();

    ASSERT_EQ(sources.load(), 1);
    ASSERT_EQ(flushes.load(), 1);
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, WaitsForFlush) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    std::promise<void> flush_started;
    std::promise<void> release_flush;
    auto released = release_flush.get_future();
    std::atomic<bool> second_returned{false};

    evget::Shutdown shutdown{scheduler, [&flush_started, &released] {
                                 flush_started.set_value();
                                 released.wait();
                             }};

    std::jthread first{[&shutdown] { shutdown.Request(); }};
    flush_started.get_future().wait();

    std::jthread second{[&shutdown, &second_returned] {
        shutdown.Request();
        second_returned.store(true);
    }};
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_FALSE(second_returned.load());

    release_flush.set_value();
    first.join();
    second.join();
    scheduler->Join();

    ASSERT_TRUE(second_returned.load());
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, SkipFlushWhenSchedulerStops) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    std::atomic<int> sources{0};
    std::atomic<int> flushes{0};

    evget::Shutdown shutdown{scheduler, [&flushes] { flushes.fetch_add(1); }};
    shutdown.SetSource([&sources] { sources.fetch_add(1); });

    const CapturedLog log{};
    scheduler->Stop();
    shutdown.Request();
    scheduler->Join();

    ASSERT_EQ(sources.load(), 1);
    ASSERT_EQ(flushes.load(), 0);
    ASSERT_EQ(
        log.Lines(),
        (std::vector<std::string>{"error scheduler already stopped, any buffered events were not flushed"})
    );
}

TEST(ShutdownTest, NoSourceFlushAndStops) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    std::atomic<int> flushes{0};

    evget::Shutdown shutdown{scheduler, [&flushes] { flushes.fetch_add(1); }};

    shutdown.Request();
    scheduler->Join();

    ASSERT_EQ(flushes.load(), 1);
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, FlushBufferedEvents) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();
    evget::DatabaseManager manager{scheduler, {store}, 100, std::chrono::seconds{600}};

    evget::Shutdown shutdown{scheduler, [&manager] { manager.Flush(); }};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    shutdown.Request();
    scheduler->Join();

    auto events = store->Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 2);
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, EmptyBufferStops) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();
    evget::DatabaseManager manager{scheduler, {store}, 100, std::chrono::seconds{600}};

    evget::Shutdown shutdown{scheduler, [&manager] { manager.Flush(); }};

    shutdown.Request();
    scheduler->Join();

    ASSERT_TRUE(store->Events().empty());
    ASSERT_TRUE(scheduler->IsStopped());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(ShutdownTest, FailedCaptureFlushesAndStops) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();
    // The store-after timer keeps the pool busy until something stops the scheduler.
    evget::DatabaseManager manager{scheduler, {store}, 100, std::chrono::seconds{600}};
    std::atomic<int> sources{0};

    evget::Shutdown shutdown{scheduler, [&manager] { manager.Flush(); }};
    shutdown.SetSource([&sources] { sources.fetch_add(1); });

    QueuedNextEvent source{std::vector<int>{}};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());
    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    const CapturedLog log{};
    auto exit_code = 0;
    shutdown.SpawnCapture(loop.Start(), exit_code);
    scheduler->Join();

    ASSERT_EQ(exit_code, 1);
    ASSERT_EQ(sources.load(), 1);
    ASSERT_TRUE(std::ranges::contains(log.Lines(), "error AsyncError: source closed"));
    auto events = store->Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 2);
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, DrainedCaptureFlushesAndStops) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();
    evget::DatabaseManager manager{scheduler, {store}, 100, std::chrono::seconds{600}};

    evget::Shutdown shutdown{scheduler, [&manager] { manager.Flush(); }};

    QueuedNextEvent source{std::vector<int>{1, 2}};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    auto exit_code = 0;
    loop.StopWhenDrained();
    shutdown.SpawnCapture(loop.Start(), exit_code);
    scheduler->Join();

    ASSERT_EQ(exit_code, 0);
    ASSERT_EQ(listener.Received(), (std::vector<int>{1, 2}));
    auto events = store->Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 1);
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, ThrowingCaptureFlushesAndStops) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto store = std::make_shared<StoreMock>();
    evget::DatabaseManager manager{scheduler, {store}, 100, std::chrono::seconds{600}};

    evget::Shutdown shutdown{scheduler, [&manager] { manager.Flush(); }};

    ThrowingNextEvent source{};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};

    ASSERT_TRUE(manager.StoreEvent(StoreMock::MakeData()).has_value());

    const CapturedLog log{};
    auto exit_code = 0;
    shutdown.SpawnCapture(loop.Start(), exit_code);
    scheduler->Join();

    ASSERT_EQ(exit_code, 1);
    ASSERT_TRUE(std::ranges::contains(log.Lines(), "error capturing input events failed: source failed"));
    auto events = store->Events();
    ASSERT_EQ(events.size(), 1);
    ASSERT_EQ(events.at(0).Entries().size(), 1);
    ASSERT_TRUE(scheduler->IsStopped());
}

TEST(ShutdownTest, NonStandardExceptionsStop) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    std::atomic<int> sources{0};

    evget::Shutdown shutdown{scheduler, [] {
                                 // NOLINTNEXTLINE(hicpp-exception-baseclass)
                                 throw 1;
                             }};
    shutdown.SetSource([&sources] {
        sources.fetch_add(1);
        // NOLINTNEXTLINE(hicpp-exception-baseclass)
        throw 1;
    });

    shutdown.Request();
    shutdown.Request();
    scheduler->Join();

    ASSERT_EQ(sources.load(), 1);
    ASSERT_TRUE(scheduler->IsStopped());
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
