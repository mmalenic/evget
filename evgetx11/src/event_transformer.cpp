#include "evgetx11/event_transformer.h"

#include <memory>
#include <utility>

#include "evget/event_transformer.h"
#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

evgetx11::EventTransformerBuilder& evgetx11::EventTransformerBuilder::PointerKey(X11Api& x_wrapper) {
    pointer_key_ = EventSwitchPointerKey{x_wrapper};
    return *this;
}

evgetx11::EventTransformerBuilder& evgetx11::EventTransformerBuilder::Touch() {
    touch_ = EventSwitchTouch{};
    return *this;
}

std::unique_ptr<evget::EventTransformer<evgetx11::InputEvent>> evgetx11::EventTransformerBuilder::Build(
    X11Api& x_wrapper
) && {
    if (pointer_key_.has_value() && touch_.has_value()) {
        return std::make_unique<EventTransformer<EventSwitchPointerKey, EventSwitchTouch>>(
            x_wrapper,
            EventSwitch{x_wrapper},
            std::move(*pointer_key_),
            *touch_
        );
    }
    if (pointer_key_.has_value()) {
        return std::make_unique<EventTransformer<EventSwitchPointerKey>>(
            x_wrapper,
            EventSwitch{x_wrapper},
            std::move(*pointer_key_)
        );
    }
    if (pointer_key_.has_value() && touch_.has_value()) {
        return std::make_unique<EventTransformer<EventSwitchTouch>>(x_wrapper, EventSwitch{x_wrapper}, *touch_);
    }

    return nullptr;
}
