#include "evgetlibinput/event_transformer.h"

#include <libinput.h>
#include <xkbcommon/xkbcommon.h>

#include <cstdint>
#include <utility>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/key.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"
#include "evget/event/mouse_scroll.h"
#include "evget/input_event.h"
#include "evget/util.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/libinput.h"
#include "evgetlibinput/xkbcommon.h"

evgetlibinput::EventTransformer::EventTransformer(
    LibInputApi& libinput_api,
    XkbCommon& xkb,
    ScreenDimensions dimensions
)
    : libinput_api_{libinput_api}, xkb_{xkb}, dimensions_{dimensions} {}

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
    auto ctx = EventContext{
        .timestamp = event.GetTimestamp(),
        .device_type = this->GetDeviceType(inner_event, event_type),
        .device_name = libinput_api_.get().GetDeviceName(*device),
        .device_uuid = device_uuid,
        .system_event = {},
    };

    auto data = evget::Data{};
    switch (event_type) {
        // xf86-input-libinput uses xf86PostMotionEventM which is mouse move:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1647
        case LIBINPUT_EVENT_POINTER_MOTION: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_POINTER_MOTION);
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

            auto builder = evget::MouseMove{};
            SetBaseFields(builder, ctx, event_time);
            builder.PositionX(libinput_api_.get().GetPointerDx(*pointer_event))
                .PositionY(libinput_api_.get().GetPointerDy(*pointer_event));

            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostMotionEventM which is mouse move:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1675
        case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

            auto builder = evget::MouseMove{};
            SetBaseFields(builder, ctx, event_time);
            SetRelativePosition(builder, device_uuid, *pointer_event);

            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEvent which is mouse click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1703
        case LIBINPUT_EVENT_POINTER_BUTTON: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_POINTER_BUTTON);
            auto* pointer_event = libinput_api_.get().GetPointerEvent(*inner_event);
            auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);
            auto button_code = libinput_api_.get().GetPointerButton(*pointer_event);
            auto action = GetButtonAction(libinput_api_.get().GetPointerButtonState(*pointer_event));

            auto builder = evget::MouseClick{};
            SetBaseFields(builder, ctx, event_time);
            builder.Button(static_cast<int>(button_code)).Action(action);
            SetButtonName(builder, button_code);

            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostProximityEventM and posts a motion event on proximity-in:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2479
        case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
            auto* tool_event = libinput_api_.get().GetTabletToolEvent(*inner_event);
            auto proximity_state = libinput_api_.get().GetTabletToolProximityState(*tool_event);

            // Only proximity-in generates a motion event, matching xf86-input-libinput behaviour.
            if (proximity_state == LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN) {
                auto event_time = libinput_api_.get().GetTabletToolTimeMicroseconds(*tool_event);
                BuildTabletToolMove(data, ctx, event_time, *tool_event);
            }
            break;
        }
        // xf86-input-libinput uses xf86PostMotionEventM via xf86libinput_post_tablet_motion which is mouse move:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2368
        case LIBINPUT_EVENT_TABLET_TOOL_AXIS: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TABLET_TOOL_AXIS);
            auto* tool_event = libinput_api_.get().GetTabletToolEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletToolTimeMicroseconds(*tool_event);

            BuildTabletToolMove(data, ctx, event_time, *tool_event);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEventP and xf86libinput_post_tablet_motion is mouse move and click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2233
        case LIBINPUT_EVENT_TABLET_TOOL_TIP: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TABLET_TOOL_TIP);
            auto* tool_event = libinput_api_.get().GetTabletToolEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletToolTimeMicroseconds(*tool_event);

            BuildTabletToolMove(data, ctx, event_time, *tool_event);

            auto click_builder = evget::MouseClick{};
            auto action = GetTipAction(libinput_api_.get().GetTabletToolTipState(*tool_event));
            SetBaseFields(click_builder, ctx, event_time);
            click_builder.Action(action);

            click_builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEventP for mouse click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2256
        case LIBINPUT_EVENT_TABLET_TOOL_BUTTON: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TABLET_TOOL_BUTTON);
            auto* tool_event = libinput_api_.get().GetTabletToolEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletToolTimeMicroseconds(*tool_event);

            auto builder = evget::MouseClick{};
            auto action = GetButtonAction(libinput_api_.get().GetTabletToolButtonState(*tool_event));
            auto button_code = libinput_api_.get().GetTabletToolButton(*tool_event);
            SetBaseFields(builder, ctx, event_time);
            SetButtonName(builder, button_code);
            builder.Button(static_cast<int>(button_code)).Action(action);

            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostButtonEvent which is mouse click:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L2526
        case LIBINPUT_EVENT_TABLET_PAD_BUTTON: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TABLET_PAD_BUTTON);
            auto* pad_event = libinput_api_.get().GetTabletPadEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletPadTimeMicroseconds(*pad_event);

            auto builder = evget::MouseClick{};
            SetBaseFields(builder, ctx, event_time);
            auto button_number = libinput_api_.get().GetTabletPadButtonNumber(*pad_event);
            auto action = GetButtonAction(libinput_api_.get().GetTabletPadButtonState(*pad_event));
            builder.Button(static_cast<int>(button_number)).Action(action);

            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostTouchEvent which matches motion and button press:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1965
        case LIBINPUT_EVENT_TOUCH_DOWN: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TOUCH_DOWN);
            auto* touch_event = libinput_api_.get().GetTouchEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTouchTimeMicroseconds(*touch_event);

            auto move_builder = evget::MouseMove{};
            auto seat_slot = libinput_api_.get().GetTouchSeatSlot(*touch_event);
            SetBaseFields(move_builder, ctx, event_time);
            move_builder.TouchId(seat_slot);
            SetTouchRelativePosition(move_builder, device_uuid, seat_slot, *touch_event);

            move_builder.Build(data);

            auto click_builder = evget::MouseClick{};
            SetBaseFields(click_builder, ctx, event_time);
            click_builder.Action(evget::ButtonAction::kPress).TouchId(seat_slot);

            click_builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostTouchEvent which matches a motion event:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1965
        case LIBINPUT_EVENT_TOUCH_MOTION: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TOUCH_MOTION);
            auto* touch_event = libinput_api_.get().GetTouchEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTouchTimeMicroseconds(*touch_event);

            auto builder = evget::MouseMove{};
            auto seat_slot = libinput_api_.get().GetTouchSeatSlot(*touch_event);
            SetBaseFields(builder, ctx, event_time);
            builder.TouchId(seat_slot);
            SetTouchRelativePosition(builder, device_uuid, seat_slot, *touch_event);

            builder.Build(data);
            break;
        }
        // xf86-input-libinput uses xf86PostTouchEvent for both UP and CANCEL which matches a button release:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1965
        case LIBINPUT_EVENT_TOUCH_CANCEL: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TOUCH_CANCEL);
            BuildTouchRelease(data, ctx, device_uuid, inner_event);
            break;
        }
        case LIBINPUT_EVENT_TOUCH_UP: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TOUCH_UP);
            BuildTouchRelease(data, ctx, device_uuid, inner_event);
            break;
        }
        // xf86-input-libinput uses xf86PostMotionEventM with scroll valuators for all scroll event types:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1922
        case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_POINTER_SCROLL_WHEEL);
            BuildScrollEvent(data, ctx, inner_event);
            break;
        }
        case LIBINPUT_EVENT_POINTER_SCROLL_FINGER: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_POINTER_SCROLL_FINGER);
            BuildScrollEvent(data, ctx, inner_event);
            break;
        }
        case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS);
            BuildScrollEvent(data, ctx, inner_event);
            break;
        }
        // xf86-input-libinput uses xf86PostKeyboardEvent which is a key event:
        // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L1724
        case LIBINPUT_EVENT_KEYBOARD_KEY: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_KEYBOARD_KEY);
            auto* keyboard_event = libinput_api_.get().GetKeyboardEvent(*inner_event);
            auto event_time = libinput_api_.get().GetKeyboardTimeMicroseconds(*keyboard_event);

            auto key_code = libinput_api_.get().GetKeyboardKey(*keyboard_event);
            auto key_state = libinput_api_.get().GetKeyboardKeyState(*keyboard_event);
            auto action = GetKeyAction(key_state);

            // Documentation states that xkb key codes are evdev key codes + 8 for X11-compatibility. See
            // https://gitlab.freedesktop.org/wlroots/wlroots/-/blob/c66a910753941c905299e62d9e31a4e90c0bbe98/types/wlr_keyboard.c#L110
            // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L53
            constexpr auto kKeyCodeOffset = 8;
            const xkb_keycode_t xkb_key = key_code + kKeyCodeOffset;

            auto key_name = xkb_.get().GetKeyName(xkb_key);
            auto character = xkb_.get().GetKeyCharacter(xkb_key);

            auto builder = evget::Key{};
            SetBaseFields(builder, ctx, event_time);
            builder.Button(static_cast<int>(key_code)).Action(action);

            if (key_name.has_value()) {
                builder.ButtonName(std::move(*key_name));
            }
            if (character.has_value()) {
                builder.Character(std::move(*character));
            }

            builder.Build(data);

            xkb_.get().UpdateKeyState(xkb_key, GetXkbDirection(key_state));
            break;
        }
        // xf86-input-libinput does not implement LIBINPUT_EVENT_TABLET_PAD_KEY, Key is the closest equivalent.
        case LIBINPUT_EVENT_TABLET_PAD_KEY: {
            ctx.system_event = EVGET_STRINGIFY(LIBINPUT_EVENT_TABLET_PAD_KEY);
            auto* pad_event = libinput_api_.get().GetTabletPadEvent(*inner_event);
            auto event_time = libinput_api_.get().GetTabletPadTimeMicroseconds(*pad_event);

            auto builder = evget::Key{};
            auto key_code = libinput_api_.get().GetTabletPadKey(*pad_event);
            auto action = GetKeyAction(libinput_api_.get().GetTabletPadKeyState(*pad_event));
            SetBaseFields(builder, ctx, event_time);
            builder.Button(static_cast<int>(key_code)).Action(action);
            SetButtonName(builder, key_code);

            builder.Build(data);
            break;
        }
        default:
            break;
    }

    return data;
}

void evgetlibinput::EventTransformer::BuildTabletToolMove(
    evget::Data& data,
    const EventContext& ctx,
    std::uint64_t event_time,
    libinput_event_tablet_tool& tool_event
) {
    auto builder = evget::MouseMove{};
    SetBaseFields(builder, ctx, event_time);
    builder.PositionX(libinput_api_.get().GetTabletToolDx(tool_event))
        .PositionY(libinput_api_.get().GetTabletToolDy(tool_event));
    builder.Build(data);
}

void evgetlibinput::EventTransformer::BuildScrollEvent(
    evget::Data& data,
    const EventContext& ctx,
    LibInputEvent& event
) {
    auto* pointer_event = libinput_api_.get().GetPointerEvent(*event);
    auto event_time = libinput_api_.get().GetPointerTimeMicroseconds(*pointer_event);

    auto builder = evget::MouseScroll{};
    SetBaseFields(builder, ctx, event_time);

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

    builder.Build(data);
}

void evgetlibinput::EventTransformer::BuildTouchRelease(
    evget::Data& data,
    const EventContext& ctx,
    const std::string& device_uuid,
    LibInputEvent& event
) {
    auto* touch_event = libinput_api_.get().GetTouchEvent(*event);
    auto event_time = libinput_api_.get().GetTouchTimeMicroseconds(*touch_event);

    // Position data is not available on TOUCH_UP events.
    auto move_builder = evget::MouseMove{};
    auto seat_slot = libinput_api_.get().GetTouchSeatSlot(*touch_event);
    SetBaseFields(move_builder, ctx, event_time);
    move_builder.TouchId(seat_slot);
    move_builder.Build(data);

    auto click_builder = evget::MouseClick{};
    SetBaseFields(click_builder, ctx, event_time);
    click_builder.Action(evget::ButtonAction::kRelease).TouchId(seat_slot);

    ClearTouchPosition(device_uuid, seat_slot);

    click_builder.Build(data);
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

xkb_key_direction evgetlibinput::EventTransformer::GetXkbDirection(libinput_key_state state) {
    if (state == LIBINPUT_KEY_STATE_PRESSED) {
        return XKB_KEY_DOWN;
    }
    return XKB_KEY_UP;
}

evget::DeviceType
evgetlibinput::EventTransformer::GetDeviceType(LibInputEvent& event, libinput_event_type event_type) const {
    if (event == nullptr) {
        return evget::DeviceType::kUnknown;
    }

    // Keyboard and tablet events unambiguously identify the device type.
    switch (event_type) {
        case LIBINPUT_EVENT_KEYBOARD_KEY:
        case LIBINPUT_EVENT_TABLET_PAD_KEY:
            return evget::DeviceType::kKeyboard;
        case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
        case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
        case LIBINPUT_EVENT_TABLET_TOOL_TIP:
        case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
        case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
        case LIBINPUT_EVENT_TABLET_PAD_RING:
        case LIBINPUT_EVENT_TABLET_PAD_STRIP:
            return evget::DeviceType::kTablet;
        default:
            break;
    }

    auto* device = this->libinput_api_.get().GetDevice(*event);
    if (device == nullptr) {
        return evget::DeviceType::kUnknown;
    }

    // For pointer and touch events, use capability ordering defined by xf86-input-libinput:
    // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L3805-3846
    // This is intentionally different to xf86-input-libinput because keyboards often report pointer capability
    // resulting in an incorrect device type.
    if (this->libinput_api_.get().GetDeviceFingerCount(*device) > 0) {
        return evget::DeviceType::kTouchpad;
    }
    if (this->libinput_api_.get().DeviceHasCapability(*device, LIBINPUT_DEVICE_CAP_TOUCH)) {
        return evget::DeviceType::kTouchscreen;
    }
    if (this->libinput_api_.get().DeviceHasCapability(*device, LIBINPUT_DEVICE_CAP_POINTER)) {
        return evget::DeviceType::kMouse;
    }

    return evget::DeviceType::kUnknown;
}
