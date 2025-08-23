#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <memory>
#include <optional>

// clang-format off
#include "evget/async/scheduler/scheduler.h"
// clang-format on

TEST(SchedulerTest, SpawnVoidTaskAndJoin) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    const std::shared_ptr<int> result{};

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

    scheduler->Spawn([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<void> {
        while (!*stopped) {
        }
        co_return;
    }(stopped));
    scheduler->Spawn([]() -> boost::asio::awaitable<void> { throw std::exception{}; }());

    scheduler->Join();
}

TEST(SchedulerTest, SpawnReturnTaskException) {
    auto stopped = std::make_shared<std::atomic<bool>>(false);
    auto scheduler = std::make_shared<evget::Scheduler>();

    scheduler->Spawn<int>([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<int> {
        while (!*stopped) {
        }
        co_return 1;
    }(stopped));
    scheduler->Spawn<int>([]() -> boost::asio::awaitable<int> { throw std::exception{}; }());

    scheduler->Join();
}

TEST(SchedulerTest, Stop) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    auto stopped = std::make_shared<std::atomic<bool>>(false);

    scheduler->Spawn([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<void> {
        while (!*stopped) {
        }
        co_return;
    }(stopped));
    scheduler->Spawn([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<void> {
        *stopped = true;
        co_return;
    }(stopped));

    scheduler->Join();
    ASSERT_TRUE(stopped);
}
