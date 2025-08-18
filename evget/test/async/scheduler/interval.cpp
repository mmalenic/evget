#include "evget/async/scheduler/interval.h"

#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>

#include <memory>

#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"

TEST(IntervalTest, Tick) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    const std::shared_ptr<evget::Interval> interval = std::make_shared<evget::Interval>(std::chrono::seconds{0});
    const std::shared_ptr<evget::Result<void>> result{};

    scheduler->Spawn(
        [](std::shared_ptr<evget::Interval> interval,
           std::shared_ptr<evget::Result<void>> result) -> boost::asio::awaitable<void> {
            *result = co_await interval->Tick();
            co_return;
        }(interval, result)
    );
    scheduler->Join();

    ASSERT_TRUE(result->has_value());
}
