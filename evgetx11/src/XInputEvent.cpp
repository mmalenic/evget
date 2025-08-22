#include "evgetx11/XInputEvent.h"

#include <evget/event/schema.h>

#include <chrono>

#include "evgetx11/XWrapper.h"

evgetx11::XInputEvent::XInputEvent(XWrapper& x_wrapper)
    : event_{x_wrapper.NextEvent()},
      timestamp_{std::chrono::system_clock::now()},
      cookie_{x_wrapper.EventData(event_)} {}

bool evgetx11::XInputEvent::HasData() const {
    return cookie_ != nullptr;
}

int evgetx11::XInputEvent::GetEventType() const {
    return cookie_->evtype;
}

const evget::TimestampType& evgetx11::XInputEvent::GetTimestamp() const {
    return timestamp_;
}

evgetx11::XInputEvent evgetx11::XInputEvent::NextEvent(XWrapper& x_wrapper) {
    return XInputEvent{x_wrapper};
}
