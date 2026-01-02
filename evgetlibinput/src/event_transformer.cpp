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
    auto inner_event = std::move(event.ViewData());
    if (inner_event == nullptr) {
        return {};
    }

    auto event_type = this->libinput_api_.get().GetEventType(*inner_event);
    switch (event_type) {
        case LIBINPUT_EVENT_POINTER_MOTION:
            auto builder = evget::MouseMove{}.Timestamp(event.GetTimestamp());

            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

            builder.Interval(event.Interval(event_time));

            std::cout << libinput_event_pointer_get_dx(pointer_event);
            break;
    }

    return evget::Data{};
}
