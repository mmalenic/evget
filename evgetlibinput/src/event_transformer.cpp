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
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

            auto builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(event.Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .PositionX(libinput_api_.get().GetPointerDx(*pointer_event))
                    .PositionY(libinput_api_.get().GetPointerDy(*pointer_event));

            break;
    }

    return evget::Data{};
}

evget::DeviceType evgetlibinput::EventTransformer::GetDeviceType(LibInputEvent& event) const {
    if (event == nullptr) {
        return evget::DeviceType::kUnknown;
    }

    auto* device = this->libinput_api_.get().GetDevice(*event);
    if (device == nullptr) {
        return evget::DeviceType::kUnknown;
    }

    // Use ordering defined by xf86-input-libinput to match X11 behaviour:
    // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L3805-3846
    if (this->libinput_api_.get().GetDeviceFingerCount(*device) > 0) {
        return evget::DeviceType::kTouchpad;
    }
    if (this->libinput_api_.get().DeviceHasCapability(*device, LIBINPUT_DEVICE_CAP_TOUCH)) {
        return evget::DeviceType::kTouchscreen;
    }
    if (this->libinput_api_.get().DeviceHasCapability(*device, LIBINPUT_DEVICE_CAP_POINTER)) {
        return evget::DeviceType::kMouse;
    }
    if (this->libinput_api_.get().DeviceHasCapability(*device, LIBINPUT_DEVICE_CAP_TABLET_TOOL) ||
        this->libinput_api_.get().DeviceHasCapability(*device, LIBINPUT_DEVICE_CAP_TABLET_PAD)) {
        return evget::DeviceType::kUnknown;
    }

    return evget::DeviceType::kKeyboard;
}
