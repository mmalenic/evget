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

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <chrono>

#include "evgetcore/Error.h"

EvgetCore::Interval::Interval(std::chrono::seconds period) : _period{period} {}

EvgetCore::asio::awaitable<EvgetCore::Result<void>> EvgetCore::Interval::tick() {
    if (!timer.has_value()) {
        timer = asio::steady_timer{co_await asio::this_coro::executor, this->period()};
    }

    auto [error] = co_await timer->async_wait(asio::as_tuple(asio::use_awaitable));
    reset();

    if (error) {
        // Changing the timer value while mid async_wait causes the value to be operation_aborted.
        if (error.value() == asio::error::operation_aborted) {
            co_return Result<void>{};
        }

        co_return Err{Error{.errorType = ErrorType::AsyncError, .message = error.message()}};
    }

    co_return Result<void>{};
}

void EvgetCore::Interval::reset() {
    if (timer.has_value()) {
        timer->expires_after(this->period());
    }
}

std::chrono::seconds EvgetCore::Interval::period() const {
    return _period;
}
