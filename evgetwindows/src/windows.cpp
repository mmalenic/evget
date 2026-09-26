#include "evgetwindows/windows.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <expected>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>

#include "evget/error.h"
#include "evgetwindows/message_window.h"
#include "evgetwindows/raw_event.h"

evgetwindows::Windows::Windows(const boost::asio::any_io_executor& executor, std::function<void()> on_stop)
    : message_window_{executor, std::move(on_stop)} {}

evget::Result<std::unique_ptr<evgetwindows::Windows>>
evgetwindows::Windows::New(const boost::asio::any_io_executor& executor, std::function<void()> on_stop) {
    auto windows = std::unique_ptr<Windows>(new Windows{executor, std::move(on_stop)});

    auto start = windows->message_window_.Start();
    if (!start.has_value()) {
        return std::unexpected(start.error());
    }

    return windows;
}

evget::Result<void> evgetwindows::Windows::Start() {
    return message_window_.Start();
}

void evgetwindows::Windows::Stop() {
    message_window_.Stop();
}

void evgetwindows::Windows::RequestStop() {
    message_window_.RequestStop();
}

// NOLINTNEXTLINE(misc-include-cleaner)
boost::asio::awaitable<std::tuple<boost::system::error_code, evgetwindows::RawEvent>>
evgetwindows::Windows::ReceiveNext() {
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
    return message_window_.Channel().async_receive(boost::asio::as_tuple(boost::asio::use_awaitable));
}
