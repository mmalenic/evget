#include "evgetlibinput/event_transformer.h"

#include <libinput.h>

#include <iostream>

#include "evget/event/data.h"
#include "evgetlibinput/libinput_api.h"

evgetlibinput::EventTransformer::EventTransformer(LibInputApi& libinput_api) : libinput_api_{libinput_api} {}

evget::Data evgetlibinput::EventTransformer::TransformEvent(InputEvent event) {
    std::cout << "HERE\n";
    if (event == nullptr) {
        return {};
    }

    auto event_type = this->libinput_api_.get().GetEventType(*event);
    switch (event_type) {
        case LIBINPUT_EVENT_POINTER_MOTION:
            auto* pointer_event = libinput_event_get_pointer_event(event.get());
            std::cout << libinput_event_pointer_get_dx(pointer_event);
            break;
    }

    return evget::Data{};
}
