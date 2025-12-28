#include "evgetlibinput/event_transformer.h"

#include <libinput.h>

#include <iostream>

#include "evget/event/data.h"
#include "evgetlibinput/libinput_api.h"

evget::Data evgetlibinput::EventTransformer::TransformEvent(InputEvent event) {
    if (event != nullptr) {
        auto* pointer_event = libinput_event_get_pointer_event(event.get());
        std::cout << libinput_event_pointer_get_dx(pointer_event);
    }
    return evget::Data{};
}
