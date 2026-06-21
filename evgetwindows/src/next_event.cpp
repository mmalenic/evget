#include "evgetwindows/next_event.h"

#include <boost/asio/awaitable.hpp>

#include <utility>

#include "evget/error.h"
#include "evget/input_event.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"

evgetwindows::NextEvent::NextEvent(WindowsApi& windows_api) : windows_api_{windows_api} {}

boost::asio::awaitable<evget::Result<evget::InputEvent<evgetwindows::RawEvent>>> evgetwindows::NextEvent::Next() const {
    co_return (co_await windows_api_.get().ReceiveNext()).transform([](auto raw) {
        return evget::InputEvent{std::move(raw)};
    });
}
