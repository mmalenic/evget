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

#include "async/coroutine/RepeatingTimer.h"

Async::asio::awaitable<Async::RepeatingTimer::Result> Async::RepeatingTimer::await(Invocable<asio::awaitable<void>> auto&& callback) {
    if (!timer.has_value()) {
        timer = asio::steady_timer{co_await asio::this_coro::executor};
    }

    co_return co_await repeat(&*timer, interval, callback);
}

Async::asio::awaitable<Async::RepeatingTimer::Result> Async::RepeatingTimer::repeat(asio::steady_timer* timer, std::chrono::seconds& interval, Invocable<asio::awaitable<void>> auto&& callback) {
    timer->expires_at(timer->expiry() + interval);

    auto [error] = co_await timer->async_wait(as_tuple(asio::use_awaitable));
    if (error) {
        if (error.value() == asio::error::operation_aborted) {
            timer = {};
            co_return Result{};
        }

        co_return Err{error};
    }

    co_await callback();

    co_return co_await repeat(timer, interval, std::forward<decltype(callback)>(callback));
}

Async::RepeatingTimer::RepeatingTimer(std::chrono::seconds interval) : interval{interval} {
}

void Async::RepeatingTimer::stop() {
    timer->cancel();
}
