#include "evget/async/scheduler/interval.h"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <chrono>

#include "evget/error.h"

evget::Interval::Interval(std::chrono::seconds period) : period_{period} {}

boost::asio::awaitable<evget::Result<void>> evget::Interval::Tick() {
    // NOLINTBEGIN(clang-analyzer-core.CallAndMessage)
    if (!timer_.has_value()) {
        timer_ = boost::asio::steady_timer{co_await boost::asio::this_coro::executor, this->Period()};
    }
    // NOLINTEND(clang-analyzer-core.CallAndMessage)

    auto [err] = co_await timer_->async_wait(boost::asio::as_tuple(boost::asio::use_awaitable));
    Reset();

    if (err) {
        // Changing the timer value while mid async_wait causes the value to be operation_aborted.
        if (err.value() == boost::asio::error::operation_aborted) {
            co_return Result<void>{};
        }

        co_return Err{Error{.error_type = ErrorType::kAsyncError, .message = err.message()}};
    }

    co_return Result<void>{};
}

void evget::Interval::Reset() {
    if (timer_.has_value()) {
        timer_->expires_after(this->Period());
    }
}

std::chrono::seconds evget::Interval::Period() const {
    return period_;
}
