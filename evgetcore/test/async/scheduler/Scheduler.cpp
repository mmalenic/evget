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

#include "evgetcore/async/scheduler/Scheduler.h"

#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <memory>
#include <optional>

TEST(SchedulerTest, SpawnVoidTaskAndJoin) {
    auto scheduler = std::make_shared<EvgetCore::Scheduler>();
    const std::shared_ptr<int> result{};

    scheduler->spawn(
        [](std::shared_ptr<int> result) -> boost::asio::awaitable<void> {
            *result = 1;
            co_return;
        }(result),
        [] {}
    );
    scheduler->join();

    ASSERT_TRUE(result != nullptr && *result == 1);
}

TEST(SchedulerTest, SpawnReturnTask) {
    auto scheduler = std::make_shared<EvgetCore::Scheduler>();
    std::optional<int> result{};

    scheduler->spawn<int>([]() -> boost::asio::awaitable<int> { co_return 1; }(), [&](auto value) { result = value; });
    scheduler->join();

    ASSERT_TRUE(result.has_value() && *result == 1);
}

TEST(SchedulerTest, SpawnVoidTaskException) {
    auto stopped = std::make_shared<std::atomic<bool>>(false);
    auto scheduler = std::make_shared<EvgetCore::Scheduler>();

    scheduler->spawn([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<void> {
        while (!*stopped) {
        }
        co_return;
    }(stopped));
    scheduler->spawn([]() -> boost::asio::awaitable<void> { throw std::exception{}; }());

    scheduler->join();
}

TEST(SchedulerTest, SpawnReturnTaskException) {
    auto stopped = std::make_shared<std::atomic<bool>>(false);
    auto scheduler = std::make_shared<EvgetCore::Scheduler>();

    scheduler->spawn<int>([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<int> {
        while (!*stopped) {
        }
        co_return 1;
    }(stopped));
    scheduler->spawn<int>([]() -> boost::asio::awaitable<int> { throw std::exception{}; }());

    scheduler->join();
}

TEST(SchedulerTest, Stop) {
    auto scheduler = std::make_shared<EvgetCore::Scheduler>();
    auto stopped = std::make_shared<std::atomic<bool>>(false);

    scheduler->spawn([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<void> {
        while (!*stopped) {
        }
        co_return;
    }(stopped));
    scheduler->spawn([](std::shared_ptr<std::atomic<bool>> stopped) -> boost::asio::awaitable<void> {
        *stopped = true;
        co_return;
    }(stopped));

    scheduler->join();
    ASSERT_TRUE(stopped);
}
