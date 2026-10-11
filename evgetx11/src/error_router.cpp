#include "evgetx11/error_router.h"

#include <X11/Xlib.h>
#include <boost/scope/scope_exit.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <cstdio>
#include <format>
#include <functional>
#include <mutex>
#include <string>
#include <utility>

template <typename Handler>
evgetx11::HandlerChain<Handler>::HandlerChain(SetHandler set_handler, Handler handler)
    : set_handler_{set_handler}, handler_{handler} {}

template <typename Handler>
void evgetx11::HandlerChain<Handler>::Install() {
    const auto current = set_handler_(handler_);
    if (current != nullptr && current == left_in_place_) {
        // The handler kept by `Restore` chains to ours, so capturing it as `previous_` would loop.
        set_handler_(current);
    } else {
        left_in_place_ = nullptr;
        if (current != handler_) {
            previous_ = current;
        }
    }
}

template <typename Handler>
void evgetx11::HandlerChain<Handler>::Restore() {
    // A handler installed after evget may chain to ours, so keep it and `previous_`.
    const auto current = set_handler_(previous_);
    if (current != handler_) {
        set_handler_(current);
        left_in_place_ = current;
    }
}

template <typename Handler>
Handler evgetx11::HandlerChain<Handler>::Previous() const {
    return previous_ == handler_ ? nullptr : previous_;
}

template class evgetx11::HandlerChain<XErrorHandler>;
template class evgetx11::HandlerChain<XIOErrorHandler>;

evgetx11::ErrorRouter::ErrorRouter(
    SetHandler set_handler,
    ErrorText error_text,
    XErrorHandler handler,
    SetIoHandler set_io_handler,
    XIOErrorHandler io_handler
)
    : error_text_{error_text}, errors_{set_handler, handler}, io_errors_{set_io_handler, io_handler} {}

void evgetx11::ErrorRouter::Register(Display* display) {
    Register(display, {});
}

void evgetx11::ErrorRouter::Register(Display* display, std::function<void()> on_connection_lost) {
    const std::scoped_lock lock{mutex_};
    if (displays_.empty()) {
        errors_.Install();
        io_errors_.Install();
    }
    ++displays_[display];
    if (on_connection_lost) {
        on_connection_lost_.insert_or_assign(display, std::move(on_connection_lost));
    }
}

void evgetx11::ErrorRouter::Unregister(Display* display) {
    const std::scoped_lock lock{mutex_};
    const auto registered = displays_.find(display);
    if (registered == displays_.end()) {
        return;
    }
    if (--registered->second == 0) {
        displays_.erase(registered);
        on_connection_lost_.erase(display);
    }
    if (!displays_.empty()) {
        return;
    }

    errors_.Restore();
    io_errors_.Restore();
}

int evgetx11::ErrorRouter::Dispatch(Display* display, XErrorEvent* event) {
    XErrorHandler previous = nullptr;
    {
        const std::scoped_lock lock{mutex_};
        if (displays_.contains(display)) {
            // This is the same size as the error handler from Xlib.
            std::array<char, BUFSIZ> text{};
            error_text_(display, event->error_code, text.data(), static_cast<int>(text.size()));
            const std::string description =
                text.front() == '\0' ? std::format("error {}", event->error_code) : text.data();
            log_.Warn(std::format("X request {}.{} failed: {}", event->request_code, event->minor_code, description));
            return 0;
        }
        previous = errors_.Previous();
    }

    // A previous handler that attached to our handler already sees the error.
    thread_local bool forwarding = false;
    if (previous == nullptr || forwarding) {
        return 0;
    }

    // The previous handler runs unlocked.
    forwarding = true;
    const boost::scope::scope_exit reset{[] { forwarding = false; }};
    return previous(display, event);
}

int evgetx11::ErrorRouter::DispatchIo(Display* display) {
    std::function<void()> on_connection_lost;
    XIOErrorHandler previous = nullptr;
    {
        const std::scoped_lock lock{mutex_};
        if (displays_.contains(display)) {
            spdlog::error("lost the connection to the X server");
            if (const auto callback = on_connection_lost_.find(display); callback != on_connection_lost_.end()) {
                on_connection_lost = callback->second;
            }
        }
        previous = io_errors_.Previous();
    }

    // Xlib's fatal handling follows, so the callback is the last chance to act on the loss.
    if (on_connection_lost) {
        on_connection_lost();
    }

    thread_local bool forwarding = false;
    if (previous == nullptr || forwarding) {
        return 0;
    }

    forwarding = true;
    const boost::scope::scope_exit reset{[] { forwarding = false; }};
    return previous(display);
}

evgetx11::ErrorRouter& evgetx11::ErrorRouter::Process() {
    static ErrorRouter router{XSetErrorHandler, XGetErrorText, OnProcessError, XSetIOErrorHandler, OnProcessIoError};
    return router;
}

int evgetx11::ErrorRouter::OnProcessError(Display* display, XErrorEvent* event) {
    return Process().Dispatch(display, event);
}

int evgetx11::ErrorRouter::OnProcessIoError(Display* display) {
    return Process().DispatchIo(display);
}

evgetx11::ErrorRegistration::ErrorRegistration(ErrorRouter& router, Display* display)
    : ErrorRegistration{router, display, {}} {}

evgetx11::ErrorRegistration::ErrorRegistration(
    ErrorRouter& router,
    Display* display,
    std::function<void()> on_connection_lost
)
    : router_{router}, display_{display} {
    router_.get().Register(display_, std::move(on_connection_lost));
}

evgetx11::ErrorRegistration::~ErrorRegistration() {
    router_.get().Unregister(display_);
}
