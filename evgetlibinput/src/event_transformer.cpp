#include "evgetlibinput/event_transformer.h"

#include <libinput.h>
#include <linux/input-event-codes.h>

#include <utility>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/key.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"
#include "evget/event/mouse_scroll.h"
#include "evget/input_event.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/libinput.h"

evgetlibinput::EventTransformer::EventTransformer(
    LibInputApi& libinput_api,
    EvdevApi& evdev_api,
    ScreenDimensions dimensions
)
    : libinput_api_{libinput_api}, evdev_api_{evdev_api}, dimensions_{dimensions} {}

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
    auto data = evget::Data{};
    switch (event_type) {
        // xf86-input-libinput uses xf86PostMotionEventM which is mouse move:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1647
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
            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostMotionEventM which is mouse move:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1675
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

            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEvent which is mouse click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1703
        case LIBINPUT_EVENT_POINTER_BUTTON: {
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);
            auto button_code = libinput_api_.get().GetPointerButton(*pointer_event);
            auto action = GetButtonAction(libinput_api_.get().GetPointerButtonState(*pointer_event));

            auto builder =
                evget::MouseClick{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Button(static_cast<int>(button_code))
                    .Action(action)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);

            const auto* button_name = evdev_api_.get().EventCodeName(EV_KEY, button_code);
            if (button_name != nullptr) {
                builder.ButtonName(button_name);
            }

            SetModifierValues(builder);
            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostMotionEventM via xf86libinput_post_tablet_motion which is mouse move:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2368
        case LIBINPUT_EVENT_TABLET_TOOL_AXIS: {
            auto* tool_event = libinput_api_.get().GetTabletToolEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletToolTimeMicroseconds(*tool_event);

            auto builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .PositionX(libinput_api_.get().GetTabletToolDx(*tool_event))
                    .PositionY(libinput_api_.get().GetTabletToolDy(*tool_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);
            SetModifierValues(builder);
            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEventP and xf86libinput_post_tablet_motion is mouse move and click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2233
        case LIBINPUT_EVENT_TABLET_TOOL_TIP: {
            auto* tool_event = libinput_api_.get().GetTabletToolEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletToolTimeMicroseconds(*tool_event);

            auto move_builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .PositionX(libinput_api_.get().GetTabletToolDx(*tool_event))
                    .PositionY(libinput_api_.get().GetTabletToolDy(*tool_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);
            SetModifierValues(move_builder);
            move_builder.Build(data);

            auto action = GetTipAction(libinput_api_.get().GetTabletToolTipState(*tool_event));
            auto click_builder =
                evget::MouseClick{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Action(action)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);

            SetModifierValues(click_builder);
            click_builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEventP for mouse click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2256
        case LIBINPUT_EVENT_TABLET_TOOL_BUTTON: {
            auto* tool_event = libinput_api_.get().GetTabletToolEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletToolTimeMicroseconds(*tool_event);
            auto button_code = libinput_api_.get().GetTabletToolButton(*tool_event);
            auto action = GetButtonAction(libinput_api_.get().GetTabletToolButtonState(*tool_event));

            auto builder =
                evget::MouseClick{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Button(static_cast<int>(button_code))
                    .Action(action)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);

            const auto* button_name = evdev_api_.get().EventCodeName(EV_KEY, button_code);
            if (button_name != nullptr) {
                builder.ButtonName(button_name);
            }

            SetModifierValues(builder);
            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEvent which is mouse click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2526
        case LIBINPUT_EVENT_TABLET_PAD_BUTTON: {
            auto* pad_event = libinput_api_.get().GetTabletPadEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletPadTimeMicroseconds(*pad_event);
            auto button_number = libinput_api_.get().GetTabletPadButtonNumber(*pad_event);
            auto action = GetButtonAction(libinput_api_.get().GetTabletPadButtonState(*pad_event));

            auto builder =
                evget::MouseClick{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Button(static_cast<int>(button_number))
                    .Action(action)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);

            SetModifierValues(builder);
            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostTouchEvent which matches motion and button press:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1965
        case LIBINPUT_EVENT_TOUCH_DOWN: {
            auto* touch_event = libinput_api_.get().GetTouchEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTouchTimeMicroseconds(*touch_event);
            auto seat_slot = libinput_api_.get().GetTouchSeatSlot(*touch_event);

            auto move_builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid)
                    .TouchId(seat_slot);
            SetModifierValues(move_builder);
            SetTouchRelativePosition(move_builder, device_uuid, seat_slot, *touch_event);
            move_builder.Build(data);

            auto click_builder =
                evget::MouseClick{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Action(evget::ButtonAction::kPress)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid)
                    .TouchId(seat_slot);

            SetModifierValues(click_builder);
            click_builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostTouchEvent which matches a motion event:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1965
        case LIBINPUT_EVENT_TOUCH_MOTION: {
            auto* touch_event = libinput_api_.get().GetTouchEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTouchTimeMicroseconds(*touch_event);
            auto seat_slot = libinput_api_.get().GetTouchSeatSlot(*touch_event);

            auto builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid)
                    .TouchId(seat_slot);
            SetModifierValues(builder);
            SetTouchRelativePosition(builder, device_uuid, seat_slot, *touch_event);
            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostTouchEvent which matches a button release
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1965
        case LIBINPUT_EVENT_TOUCH_UP: {
            auto* touch_event = libinput_api_.get().GetTouchEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTouchTimeMicroseconds(*touch_event);
            auto seat_slot = libinput_api_.get().GetTouchSeatSlot(*touch_event);

            // Position data is not available on TOUCH_UP events.
            auto move_builder =
                evget::MouseMove{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid)
                    .TouchId(seat_slot);
            SetModifierValues(move_builder);
            move_builder.Build(data);

            auto click_builder =
                evget::MouseClick{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Action(evget::ButtonAction::kRelease)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid)
                    .TouchId(seat_slot);

            SetModifierValues(click_builder);
            click_builder.Build(data);
            ClearTouchPosition(device_uuid, seat_slot);
            break;
        }
        // xf86-input-libinput uses xf86PostMotionEventM with scroll valuators for all scroll event types:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1922
        case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
        case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
        case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS: {
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

            auto builder =
                evget::MouseScroll{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);

            if (libinput_api_.get().GetPointerHasAxis(*pointer_event, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
                builder.Vertical(
                    libinput_api_.get().GetPointerScrollValue(*pointer_event, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)
                );
            }
            if (libinput_api_.get().GetPointerHasAxis(*pointer_event, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
                builder.Horizontal(
                    libinput_api_.get().GetPointerScrollValue(*pointer_event, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)
                );
            }

            SetModifierValues(builder);
            builder.Build(data);
            break;
        }
        // xf86-input-libinput does not implement LIBINPUT_EVENT_TABLET_PAD_KEY, Key is the closest equivalent.
        case LIBINPUT_EVENT_TABLET_PAD_KEY: {
            auto* pad_event = libinput_api_.get().GetTabletPadEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletPadTimeMicroseconds(*pad_event);
            auto key_code = libinput_api_.get().GetTabletPadKey(*pad_event);
            auto action = GetKeyAction(libinput_api_.get().GetTabletPadKeyState(*pad_event));

            auto builder =
                evget::Key{}
                    .Timestamp(event.GetTimestamp())
                    .Interval(device_intervals_[device_uuid].Interval(event_time))
                    .Device(this->GetDeviceType(inner_event))
                    .Button(static_cast<int>(key_code))
                    .Action(action)
                    .DeviceName(libinput_api_.get().GetDeviceName(*device))
                    .DeviceId(device_uuid);

            const auto* key_name = evdev_api_.get().EventCodeName(EV_KEY, key_code);
            if (key_name != nullptr) {
                builder.ButtonName(key_name);
            }

            SetModifierValues(builder);
            builder.Build(data);
            break;
        }
    }

    return data;
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

void evgetlibinput::EventTransformer::SetTouchRelativePosition(
    evget::MouseMove& builder,
    const std::string& device_uuid,
    std::int32_t seat_slot,
    libinput_event_touch& touch_event
) {
    // Each relative motion stream for touch events must take into account the seat slot to support
    // multiple touch points.
    auto key = std::make_pair(device_uuid, seat_slot);
    auto absolute_x = libinput_api_.get().GetTouchX(touch_event, dimensions_.width);
    auto absolute_y = libinput_api_.get().GetTouchY(touch_event, dimensions_.height);

    // Convert absolute motion to relative motion so that there is consistency with
    // `LIBINPUT_EVENT_POINTER_MOTION`.
    if (previous_touch_x_.contains(key) && previous_touch_y_.contains(key)) {
        builder.PositionX(absolute_x - previous_touch_x_[key]).PositionY(absolute_y - previous_touch_y_[key]);
    }

    previous_touch_x_[key] = absolute_x;
    previous_touch_y_[key] = absolute_y;
}

void evgetlibinput::EventTransformer::ClearTouchPosition(const std::string& device_uuid, std::int32_t seat_slot) {
    auto key = std::make_pair(device_uuid, seat_slot);
    previous_touch_x_.erase(key);
    previous_touch_y_.erase(key);
}

evget::ButtonAction evgetlibinput::EventTransformer::GetButtonAction(libinput_button_state state) {
    if (state == LIBINPUT_BUTTON_STATE_PRESSED) {
        return evget::ButtonAction::kPress;
    }
    return evget::ButtonAction::kRelease;
}

evget::ButtonAction evgetlibinput::EventTransformer::GetTipAction(libinput_tablet_tool_tip_state state) {
    if (state == LIBINPUT_TABLET_TOOL_TIP_DOWN) {
        return evget::ButtonAction::kPress;
    }
    return evget::ButtonAction::kRelease;
}

evget::ButtonAction evgetlibinput::EventTransformer::GetKeyAction(libinput_key_state state) {
    if (state == LIBINPUT_KEY_STATE_PRESSED) {
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
        return evget::DeviceType::kTablet;
    }

    return evget::DeviceType::kKeyboard;
}
