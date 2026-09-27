#include "evgetx11/error_router.h"

#include <X11/Xlib.h>

#include <array>
#include <cstdio>
#include <format>
#include <mutex>
#include <string>

evgetx11::ErrorRouter::ErrorRouter(SetHandler set_handler, ErrorText error_text, XErrorHandler handler)
    : set_handler_{set_handler}, error_text_{error_text}, handler_{handler} {}

void evgetx11::ErrorRouter::Register(Display* display) {
    const std::scoped_lock lock{mutex_};
    if (displays_.empty()) {
        previous_ = set_handler_(handler_);
    }
    ++displays_[display];
}

void evgetx11::ErrorRouter::Unregister(Display* display) {
    const std::scoped_lock lock{mutex_};
    const auto registered = displays_.find(display);
    if (registered == displays_.end()) {
        return;
    }
    if (--registered->second == 0) {
        displays_.erase(registered);
    }
    if (!displays_.empty()) {
        return;
    }

    // Leave any handler an embedding program installed after evget.
    const auto current = set_handler_(previous_);
    if (current != handler_) {
        set_handler_(current);
    }
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
        previous = previous_;
    }

    if (previous == nullptr || previous == handler_) {
        return 0;
    }
    return previous(display, event);
}

evgetx11::ErrorRouter& evgetx11::ErrorRouter::Process() {
    static ErrorRouter router{XSetErrorHandler, XGetErrorText, OnProcessError};
    return router;
}

int evgetx11::ErrorRouter::OnProcessError(Display* display, XErrorEvent* event) {
    return Process().Dispatch(display, event);
}

evgetx11::ErrorRegistration::ErrorRegistration(ErrorRouter& router, Display* display)
    : router_{router}, display_{display} {
    router_.get().Register(display_);
}

evgetx11::ErrorRegistration::~ErrorRegistration() {
    router_.get().Unregister(display_);
}
