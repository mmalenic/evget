#include "common/event_loop.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>

#include "evget/error.h"
#include "evget/event_loop.h"

test::QueuedNextEvent::QueuedNextEvent(std::vector<int> events) : events_{events.begin(), events.end()} {}

boost::asio::awaitable<evget::Result<int>> test::QueuedNextEvent::Next() const {
    std::optional<int> event{};
    {
        const std::scoped_lock lock{mutex_};
        if (!events_.empty()) {
            event = events_.front();
            events_.pop_front();
        }
    }

    if (!event.has_value()) {
        co_return evget::Err{{.error_type = evget::ErrorType::kAsyncError, .message = "source closed"}};
    }
    co_return *event;
}

boost::asio::awaitable<evget::Result<int>> test::ThrowingNextEvent::Next() const {
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    co_await boost::asio::this_coro::executor;
    co_return []() -> evget::Result<int> { throw std::runtime_error{"source failed"}; }();
}

// Boost.Asio's has a false position with use-after-free here.
test::PendingNextEvent::PendingNextEvent(const boost::asio::any_io_executor& executor)
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
    : timer_{executor, boost::asio::steady_timer::time_point::max()} {}

boost::asio::awaitable<evget::Result<int>> test::PendingNextEvent::Next() const {
    called_.store(true);
    called_.notify_all();
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
    co_await timer_.async_wait(boost::asio::use_awaitable);
    co_return evget::Err{{.error_type = evget::ErrorType::kAsyncError, .message = "source destroyed"}};
}

void test::PendingNextEvent::WaitForNext() const {
    called_.wait(false);
}

boost::asio::awaitable<evget::Result<void>> test::RecordingListener::Notify(int event) {
    const std::scoped_lock lock{mutex_};
    received_.push_back(event);
    co_return evget::Result<void>{};
}

boost::asio::awaitable<evget::Result<void>> test::RecordingListener::Start() {
    co_return evget::Result<void>{};
}

std::vector<int> test::RecordingListener::Received() {
    const std::scoped_lock lock{mutex_};
    return received_;
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
boost::asio::awaitable<void> test::RunLoop(evget::EventLoop<int>& loop, std::optional<evget::Result<void>>& result) {
    result = co_await loop.Start();
}
