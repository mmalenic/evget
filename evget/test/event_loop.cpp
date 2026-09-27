#include "evget/event_loop.h"

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <optional>
#include <vector>

#include "common/event_loop.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace {

using test::QueuedNextEvent;
using test::RecordingListener;
using test::RunLoop;

} // namespace

TEST(EventLoopTest, StopQueuedEvents) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    QueuedNextEvent source{std::vector<int>{1, 2, 3}};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};
    std::optional<evget::Result<void>> result{};

    loop.Stop();
    scheduler->Spawn(RunLoop(loop, result), [] {});
    const auto finished = loop.WaitFinished(std::chrono::seconds{1});
    scheduler->Stop();
    scheduler->Join();

    ASSERT_TRUE(finished);
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(result->has_value());
    ASSERT_EQ(listener.Received(), (std::vector<int>{1, 2, 3}));
}

TEST(EventLoopTest, StopSourceExits) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    QueuedNextEvent source{std::vector<int>{}};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};
    std::optional<evget::Result<void>> result{};

    loop.Stop();
    scheduler->Spawn(RunLoop(loop, result), [] {});
    const auto finished = loop.WaitFinished(std::chrono::seconds{1});
    scheduler->Stop();
    scheduler->Join();

    ASSERT_TRUE(finished);
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(result->has_value());
    ASSERT_TRUE(listener.Received().empty());
}

TEST(EventLoopTest, StopExits) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    QueuedNextEvent source{std::vector<int>{1, 2, 3}};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};
    std::optional<evget::Result<void>> result{};

    loop.Stop();
    scheduler->Spawn(RunLoop(loop, result), [] {});
    const auto finished = loop.WaitFinished(std::chrono::seconds{1});
    scheduler->Stop();
    scheduler->Join();

    ASSERT_TRUE(finished);
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(result->has_value());
    ASSERT_TRUE(listener.Received().empty());
}

TEST(EventLoopTest, SourceClosedNoStop) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    QueuedNextEvent source{std::vector<int>{1}};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};
    std::optional<evget::Result<void>> result{};

    scheduler->Spawn(RunLoop(loop, result), [] {});
    const auto finished = loop.WaitFinished(std::chrono::seconds{1});
    scheduler->Stop();
    scheduler->Join();

    ASSERT_TRUE(finished);
    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_FALSE(result->has_value());
    ASSERT_EQ(listener.Received(), (std::vector<int>{1}));
}

TEST(EventLoopTest, WaitTimesOut) {
    QueuedNextEvent source{std::vector<int>{}};
    RecordingListener listener;
    evget::EventLoop<int> loop{source, {listener}};

    ASSERT_FALSE(loop.WaitFinished(std::chrono::milliseconds{10}));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
