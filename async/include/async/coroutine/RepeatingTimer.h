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

#ifndef TIMER_H
#define TIMER_H

#include <boost/asio.hpp>

#include <iostream>

#include "async/Util.h"

namespace Async {

namespace asio = boost::asio;

/**
 * \brief A repeating timer indefinitely repeats until it is stopped.
 */
class RepeatingTimer {
public:
    /**
     * \brief construct a repeating timer.
     * \param interval interval for repeat
     */
    explicit RepeatingTimer(std::chrono::seconds interval);

    /**
     * \brief Await on the timer. Extends any existing awaits with the interval value.
     * This means that callbacks on existing awaits are delayed until the new interval
     * is reached.
     *
     * \param callback the callback.
     * \return resulting indicating success or failure.
     */
    asio::awaitable<Result<void, boost::system::error_code>> await(Invocable<asio::awaitable<void>> auto&& callback);

    /**
     * \brief Stop the timer. Callbacks on existing awaits are run but only after the
     * timer expires.
     */
    void stop();

private:
    using timepoint = asio::steady_timer::clock_type::time_point;

    asio::awaitable<Result<void, boost::system::error_code>> repeat(Invocable<asio::awaitable<void>> auto&& callback);
    asio::awaitable<Result<timepoint, boost::system::error_code>> awaitRepeat(Invocable<asio::awaitable<void>> auto&& callback);

    std::chrono::seconds interval;
    std::optional<asio::steady_timer> timer;
};

asio::awaitable<Result<void, boost::system::error_code>> RepeatingTimer::await(Invocable<asio::awaitable<void>> auto&& callback) {
    if (!timer.has_value()) {
        timer = asio::steady_timer{co_await asio::this_coro::executor};
    }

    co_return co_await repeat(callback);
}

asio::awaitable<Result<void, boost::system::error_code>> RepeatingTimer::repeat(Invocable<asio::awaitable<void>> auto&& callback) {
    auto time = timepoint::min();

    // Start with one loop and only repeat until the timer is cancelled via a min timepoint.
    do {
        auto result = co_await awaitRepeat(callback);
        if (!result.has_value()) {
            co_return Err{result.error()};
        }

        time = result.value();
    } while (time != timepoint::min());

    // Timer has been stopped.
    co_return Result<void, boost::system::error_code>{};
}

asio::awaitable<Result<RepeatingTimer::timepoint, boost::system::error_code>> RepeatingTimer::awaitRepeat(
    Invocable<asio::awaitable<void>> auto&& callback
    ) {
    // Set the correct expirey time, even if the timer is already in progress.
    timer->expires_after(interval);

    // Multiple threads are allowed to async_wait on the same timer.
    auto [error] = co_await timer->async_wait(as_tuple(asio::use_awaitable));
    // Todo solve race condition here.
    auto time = timer->expiry();

    if (error) {
        // Changing the timer value while mid async_wait causes the value to be operation_aborted.
        if (error.value() == asio::error::operation_aborted) {
            co_return Result<timepoint, boost::system::error_code>{time};
        }

        co_return Err{error};
    }

    if (time != timepoint::min()) {
        co_await callback();
    }

    co_return Result<timepoint, boost::system::error_code>{time};
}
}

#endif //TIMER_H
