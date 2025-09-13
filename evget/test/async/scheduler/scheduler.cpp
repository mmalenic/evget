#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>

// clang-format off
#include "evget/async/scheduler/scheduler.h"

#include "evget/async/scheduler/interval.h"
// clang-format on

TEST(SchedulerTest, SpawnVoidTaskAndJoin) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    const auto result = std::make_shared<int>();

    scheduler->Spawn(
        [](std::shared_ptr<int> result) -> boost::asio::awaitable<void> {
            *result = 1;
            co_return;
        }(result),
        [] {}
    );
    scheduler->Join();

    ASSERT_TRUE(result != nullptr && *result == 1);
}

TEST(SchedulerTest, SpawnReturnTask) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    std::optional<int> result{};

    scheduler->Spawn<int>([]() -> boost::asio::awaitable<int> { co_return 1; }(), [&](auto value) { result = value; });
    scheduler->Join();

    ASSERT_TRUE(result.has_value() && *result == 1);
}

TEST(SchedulerTest, SpawnVoidTaskException) {
    auto stopped = std::make_shared<std::atomic<bool>>(false);
    auto scheduler = std::make_shared<evget::Scheduler>();

    const auto interval_one = std::make_shared<evget::Interval>(std::chrono::seconds{1});
    scheduler->Spawn(
        [](std::shared_ptr<std::atomic<bool>> stopped,
           std::shared_ptr<evget::Interval> interval) -> boost::asio::awaitable<void> {
            while (!*stopped) {
                co_await interval->Tick();
            }
            co_return;
        }(stopped, interval_one)
    );

    const auto interval_two = std::make_shared<evget::Interval>(std::chrono::seconds{1});
    scheduler->Spawn([](std::shared_ptr<evget::Interval> interval) -> boost::asio::awaitable<void> {
        // NOLINTBEGIN(clang-analyzer-core.CallAndMessage)
        co_await interval->Tick();
        // NOLINTEND(clang-analyzer-core.CallAndMessage)
        throw std::exception{};
    }(interval_two));

    scheduler->Join();
}

TEST(SchedulerTest, SpawnReturnTaskException) {
    auto stopped = std::make_shared<std::atomic<bool>>(false);
    auto scheduler = std::make_shared<evget::Scheduler>();

    const auto interval_one = std::make_shared<evget::Interval>(std::chrono::seconds{1});
    scheduler->Spawn<int>(
        [](std::shared_ptr<std::atomic<bool>> stopped,
           std::shared_ptr<evget::Interval> interval) -> boost::asio::awaitable<int> {
            while (!*stopped) {
                co_await interval->Tick();
            }
            co_return 1;
        }(stopped, interval_one)
    );

    const auto interval_two = std::make_shared<evget::Interval>(std::chrono::seconds{1});
    scheduler->Spawn<int>([](std::shared_ptr<evget::Interval> interval) -> boost::asio::awaitable<int> {
        // NOLINTBEGIN(clang-analyzer-core.CallAndMessage)
        co_await interval->Tick();
        // NOLINTEND(clang-analyzer-core.CallAndMessage)
        throw std::exception{};
    }(interval_two));

    scheduler->Join();
}

TEST(SchedulerTest, Stop) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto stopped = std::make_shared<std::atomic<bool>>(false);
    const auto interval = std::make_shared<evget::Interval>(std::chrono::seconds{1});

    scheduler->Spawn(
        [](std::shared_ptr<std::atomic<bool>> stopped,
           std::shared_ptr<evget::Interval> interval) -> boost::asio::awaitable<void> {
            while (!*stopped) {
                co_await interval->Tick();
            }
            co_return;
        }(stopped, interval)
    );
    scheduler->Spawn(
        [](std::shared_ptr<std::atomic<bool>> stopped,
           std::shared_ptr<evget::Interval> interval) -> boost::asio::awaitable<void> {
            while (!*stopped) {
                co_await interval->Tick();
            }
            co_return;
        }(stopped, interval)
    );
    scheduler->Stop();

    scheduler->Join();
}
