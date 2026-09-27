#include "evgetx11/input_event.h"

#include <X11/Xlib.h>
#include <evget/event/schema.h>

#include <memory>
#include <utility>

#include "evget/input_event.h"
#include "evgetx11/x11.h"

evgetx11::InputEvent::InputEvent(X11Api& x_wrapper)
    : event_{std::make_unique<evget::InputEvent<XEvent>>(x_wrapper.NextEvent())},
      cookie_{x_wrapper.EventData(event_->ViewData())} {}

evgetx11::InputEvent& evgetx11::InputEvent::operator=(InputEvent&& other) noexcept {
    // The old cookie must be freed first while the event exists.
    cookie_ = std::move(other.cookie_);
    event_ = std::move(other.event_);
    return *this;
}

bool evgetx11::InputEvent::HasData() const {
    return cookie_ != nullptr;
}

int evgetx11::InputEvent::GetEventType() const {
    return cookie_->evtype;
}

const evget::TimestampType& evgetx11::InputEvent::GetTimestamp() const {
    return event_->GetTimestamp();
}

evgetx11::InputEvent evgetx11::InputEvent::NextEvent(X11Api& x_wrapper) {
    return InputEvent{x_wrapper};
}
