#include "evgetlibinput/event_transformer.h"

#include <libinput.h>

#include <utility>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"
#include "evget/input_event.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/libinput.h"

evgetlibinput::EventTransformer::EventTransformer(LibInputApi& libinput_api, evgetlibinput::ScreenDimensions dimensions)
    : libinput_api_{libinput_api}, dimensions_{dimensions} {}

evget::Data evgetlibinput::EventTransformer::TransformEvent(evget::InputEvent<LibInputEvent> event) {
    auto inner_event = std::move(event.ViewData());
    if (inner_event == nullptr) {
        return {};
    }

    auto* device = this->libinput_api_.get().GetDevice(*inner_event);
    if (device == nullptr) {
        return {};
    }

    const auto& device_uuid = device_ids_.Uuid(device);
    auto event_type = this->libinput_api_.get().GetEventType(*inner_event);
    switch (event_type) {
        case LIBINPUT_EVENT_POINTER_MOTION: {
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

            auto builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .PositionX(libinput_api_.get().GetPointerDx(*pointer_event))
                    .PositionY(libinput_api_.get().GetPointerDy(*pointer_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);
            SetModifierValues(builder);
            break;
        }
        case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE: {
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

            auto builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);
            SetModifierValues(builder);

            SetRelativePosition(builder, device_uuid, *pointer_event);

            break;
        }
        case LIBINPUT_EVENT_POINTER_BUTTON: {
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);
            auto action = GetButtonAction(libinput_api_.get().GetPointerButtonState(*pointer_event));

            auto builder =
                evget::MouseClick{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Button(static_cast<int>(libinput_api_.get().GetPointerButton(*pointer_event)))
                    .Action(action)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);
            SetModifierValues(builder);
            break;
        }
    }

    return evget::Data{};
}

void evgetlibinput::EventTransformer::SetRelativePosition(
    evget::MouseMove& builder,
    const std::string& device_uuid,
    libinput_event_pointer& pointer_event
) {
    auto absolute_x = libinput_api_.get().GetPointerAbsoluteX(pointer_event, dimensions_.width);
    auto absolute_y = libinput_api_.get().GetPointerAbsoluteY(pointer_event, dimensions_.height);

    // Convert absolute motion to relative motion so that there is consistency with
    // `LIBINPUT_EVENT_POINTER_MOTION`.
    if (previous_absolute_x_.contains(device_uuid) && previous_absolute_y_.contains(device_uuid)) {
        builder.PositionX(absolute_x - previous_absolute_x_[device_uuid])
            .PositionY(absolute_y - previous_absolute_y_[device_uuid]);
    }

    previous_absolute_x_[device_uuid] = absolute_x;
    previous_absolute_y_[device_uuid] = absolute_y;
}

evget::ButtonAction evgetlibinput::EventTransformer::GetButtonAction(libinput_button_state state) {
    if (state == LIBINPUT_BUTTON_STATE_PRESSED) {
        return evget::ButtonAction::kPress;
    }
    return evget::ButtonAction::kRelease;
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
