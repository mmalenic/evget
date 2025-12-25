#include "evgetlibinput/event_transformer.h"

#include "evget/event/data.h"
#include "evgetlibinput/libinput_api.h"

evget::Data evgetlibinput::EventTransformer::TransformEvent(InputEvent /*event*/) {
    return evget::Data{};
}
