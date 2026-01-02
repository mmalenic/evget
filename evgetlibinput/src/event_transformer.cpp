#include "evgetlibinput/event_transformer.h"

#include <libinput.h>

#include <iostream>
#include <utility>

#include "evget/event/data.h"
#include "evget/event/mouse_move.h"
#include "evget/input_event.h"
#include "evgetlibinput/libinput_api.h"

evgetlibinput::EventTransformer::EventTransformer(LibInputApi& libinput_api) : libinput_api_{libinput_api} {}

evget::Data evgetlibinput::EventTransformer::TransformEvent(evget::InputEvent<LibInputEvent> event) {
    auto [timestamp, inner_event] = std::move(event).IntoInner();
    if (inner_event == nullptr) {
        return {};
    }

    auto event_type = this->libinput_api_.get().GetEventType(*inner_event);
    switch (event_type) {
        case LIBINPUT_EVENT_POINTER_MOTION:
            auto builder = evget::MouseMove{}.Timestamp(timestamp);

            auto* pointer_event = libinput_event_get_pointer_event(inner_event.get());
            std::cout << libinput_event_pointer_get_dx(pointer_event);
            break;
    }

    return evget::Data{};
}
