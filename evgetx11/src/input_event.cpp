#include "evgetx11/input_event.h"

#include <evget/event/schema.h>

#include <chrono>

#include "evgetx11/x11_api.h"

evgetx11::InputEvent::InputEvent(X11Api& x_wrapper)
    : event_{x_wrapper.NextEvent()}, timestamp_{evget::Now()}, cookie_{x_wrapper.EventData(event_)} {}

bool evgetx11::InputEvent::HasData() const {
    return cookie_ != nullptr;
}

int evgetx11::InputEvent::GetEventType() const {
    return cookie_->evtype;
}

const evget::TimestampType& evgetx11::InputEvent::GetTimestamp() const {
    return timestamp_;
}

evgetx11::InputEvent evgetx11::InputEvent::NextEvent(X11Api& x_wrapper) {
    return InputEvent{x_wrapper};
}
