#include "evgetx11/event_loop.h"

#include <boost/asio/awaitable.hpp>

#include <utility>

#include "evget/error.h"
#include "evget/event_listener.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"

boost::asio::awaitable<bool> evgetx11::EventLoop::IsStopped() {
    co_return stopped_.load();
}

boost::asio::awaitable<evget::Result<void>> evgetx11::EventLoop::Start() {
    while (!co_await IsStopped()) {
        auto result = this->Notify(handler_.GetEvent());
        if (!result.has_value()) {
            co_return evget::Err{result.error()};
        }
    }
    co_return evget::Result<void>{};
}

void evgetx11::EventLoop::Stop() {
    stopped_.store(true);
}

void evgetx11::EventLoop::RegisterEventListener(evget::EventListener<InputEvent>& event_listener) {
    event_listener_ = event_listener;
}

evget::Result<void> evgetx11::EventLoop::Notify(InputEvent event) {
    if (event_listener_.has_value()) {
        return event_listener_->get().Notify(std::move(event));
    }
    return {};
}

evgetx11::EventLoop::EventLoop(InputHandler x_input_handler) : handler_{x_input_handler} {}
