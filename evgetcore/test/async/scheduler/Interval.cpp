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

#include "evgetcore/async/scheduler/Interval.h"

#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>

#include <memory>

#include "evgetcore/Error.h"
#include "evgetcore/async/scheduler/Scheduler.h"

TEST(IntervalTest, Tick) {
    auto scheduler = std::make_shared<EvgetCore::Scheduler>();
    const std::shared_ptr<EvgetCore::Interval> interval =
        std::make_shared<EvgetCore::Interval>(std::chrono::seconds{0});
    const std::shared_ptr<EvgetCore::Result<void>> result{};

    scheduler->spawn(
        [](std::shared_ptr<EvgetCore::Interval> interval,
           std::shared_ptr<EvgetCore::Result<void>> result) -> boost::asio::awaitable<void> {
            *result = co_await interval->tick();
            co_return;
        }(interval, result)
    );
    scheduler->join();

    ASSERT_TRUE(result->has_value());
}
