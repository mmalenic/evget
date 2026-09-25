#include "common/event_loop.h"

#include <boost/asio/awaitable.hpp>

#include <mutex>
#include <optional>
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
