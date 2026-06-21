#include "evgetwindows/windows.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <expected>
#include <memory>
#include <utility>

#include "evget/error.h"
#include "evgetwindows/message_window.h"
#include "evgetwindows/raw_event.h"

evgetwindows::Windows::Windows(boost::asio::any_io_executor executor) : message_window_{std::move(executor)} {}

evget::Result<std::unique_ptr<evgetwindows::Windows>> evgetwindows::Windows::New(
    boost::asio::any_io_executor executor
) {
    auto windows = std::unique_ptr<Windows>(new Windows{std::move(executor)});

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

boost::asio::awaitable<evget::Result<evgetwindows::RawEvent>> evgetwindows::Windows::ReceiveNext() {
    auto [error, raw] = co_await message_window_.Channel().async_receive(boost::asio::as_tuple(boost::asio::use_awaitable));
    if (error) {
        co_return evget::Err{{.error_type = evget::ErrorType::kAsyncError, .message = error.message()}};
    }
    co_return raw;
}
