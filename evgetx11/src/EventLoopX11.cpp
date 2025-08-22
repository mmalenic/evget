#include "evgetx11/EventLoopX11.h"

#include <boost/asio/awaitable.hpp>

#include <utility>

#include "evget/error.h"
#include "evget/event_listener.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XInputHandler.h"

boost::asio::awaitable<bool> evgetx11::EventLoopX11::IsStopped() {
    co_return stopped_.load();
}

boost::asio::awaitable<evget::Result<void>> evgetx11::EventLoopX11::Start() {
    while (!co_await IsStopped()) {
        auto result = this->Notify(handler_.GetEvent());
        if (!result.has_value()) {
            co_return evget::Err{result.error()};
        }
    }
    co_return evget::Result<void>{};
}

void evgetx11::EventLoopX11::Stop() {
    stopped_.store(true);
}

void evgetx11::EventLoopX11::RegisterEventListener(evget::EventListener<XInputEvent>& event_listener) {
    event_listener_ = event_listener;
}

evget::Result<void> evgetx11::EventLoopX11::Notify(XInputEvent event) {
    if (event_listener_.has_value()) {
        return event_listener_->get().Notify(std::move(event));
    }
    return {};
}

evgetx11::EventLoopX11::EventLoopX11(XInputHandler x_input_handler) : handler_{x_input_handler} {}
