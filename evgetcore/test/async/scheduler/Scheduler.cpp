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

#include <gtest/gtest.h>

#include "evgetcore/async/scheduler/Scheduler.h"

TEST(SchedulerTest, SpawnVoidTaskAndJoin) {  // NOLINT(cert-err58-cpp)
    EvgetCore::Scheduler scheduler{};
    std::optional<int> result{};

    scheduler.spawn([&]() -> boost::asio::awaitable<void> {
        result = 1;
        co_return;
    }, [&] {
    });
    scheduler.join();

    ASSERT_EQ(*result, 1);
}

TEST(SchedulerTest, SpawnReturnTask) {  // NOLINT(cert-err58-cpp)
    EvgetCore::Scheduler scheduler{};
    std::optional<int> result{};

    scheduler.spawn<int>([&]() -> boost::asio::awaitable<int> {
        co_return 1;
    }, [&](auto value) {
        result = value;
    });
    scheduler.join();

    ASSERT_EQ(*result, 1);
}

TEST(SchedulerTest, SpawnVoidTaskException) {  // NOLINT(cert-err58-cpp)
    EvgetCore::Scheduler scheduler{};

    scheduler.spawn([&]() -> boost::asio::awaitable<void> {
        while (!co_await scheduler.isStopped()) {
        }
    });
    scheduler.spawn([&]() -> boost::asio::awaitable<void> {
        throw std::exception{};
    });

    scheduler.join();
}

TEST(SchedulerTest, SpawnReturnTaskException) {  // NOLINT(cert-err58-cpp)
    EvgetCore::Scheduler scheduler{};

    scheduler.spawn<int>([&]() -> boost::asio::awaitable<int> {
        while (!co_await scheduler.isStopped()) {
        }
        co_return 1;
    });
    scheduler.spawn<int>([&]() -> boost::asio::awaitable<int> {
        throw std::exception{};
    });

    scheduler.join();
}

TEST(SchedulerTest, Stop) {  // NOLINT(cert-err58-cpp)
    EvgetCore::Scheduler scheduler{};
    bool stopped{};

    scheduler.spawn([&]() -> boost::asio::awaitable<void> {
        while (!co_await scheduler.isStopped()) {
        }
    });
    scheduler.spawn([&]() -> boost::asio::awaitable<void> {
        scheduler.stop();
        stopped = co_await scheduler.isStopped();
    });

    scheduler.join();
    ASSERT_TRUE(stopped);
}