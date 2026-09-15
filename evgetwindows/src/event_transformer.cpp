#include "evgetwindows/event_transformer.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/key.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"
#include "evget/event/mouse_scroll.h"
#include "evget/event/schema.h"
#include "evget/input_event.h"
#include "evget/util.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_query_api.h"
#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/vk_keysym.h"
#include "evgetwindows/windows_query_api.h"

namespace {

// These match the libinput button ids from input-event-codes.h as windows has no concept of a button id.
constexpr int kButtonIdLeft = 0x110;
constexpr int kButtonIdRight = 0x111;
constexpr int kButtonIdMiddle = 0x112;
constexpr int kButtonIdSide = 0x113;
constexpr int kButtonIdExtra = 0x114;

struct MouseButton {
    USHORT down_flag;
    USHORT up_flag;
    int button_id;
    std::string_view name;
    std::string_view down_event;
    std::string_view up_event;
};

struct ButtonTransition {
    USHORT flag;
    std::string_view system_event;
    evget::ButtonAction action;
};

constexpr std::array<MouseButton, 5> kMouseButtons{{
    {.down_flag = RI_MOUSE_LEFT_BUTTON_DOWN,
     .up_flag = RI_MOUSE_LEFT_BUTTON_UP,
     .button_id = kButtonIdLeft,
     .name = "BTN_LEFT",
     .down_event = EVGET_STRINGIFY(RI_MOUSE_LEFT_BUTTON_DOWN),
     .up_event = EVGET_STRINGIFY(RI_MOUSE_LEFT_BUTTON_UP)},
    {.down_flag = RI_MOUSE_RIGHT_BUTTON_DOWN,
     .up_flag = RI_MOUSE_RIGHT_BUTTON_UP,
     .button_id = kButtonIdRight,
     .name = "BTN_RIGHT",
     .down_event = EVGET_STRINGIFY(RI_MOUSE_RIGHT_BUTTON_DOWN),
     .up_event = EVGET_STRINGIFY(RI_MOUSE_RIGHT_BUTTON_UP)},
    {.down_flag = RI_MOUSE_MIDDLE_BUTTON_DOWN,
     .up_flag = RI_MOUSE_MIDDLE_BUTTON_UP,
     .button_id = kButtonIdMiddle,
     .name = "BTN_MIDDLE",
     .down_event = EVGET_STRINGIFY(RI_MOUSE_MIDDLE_BUTTON_DOWN),
     .up_event = EVGET_STRINGIFY(RI_MOUSE_MIDDLE_BUTTON_UP)},
    {.down_flag = RI_MOUSE_BUTTON_4_DOWN,
     .up_flag = RI_MOUSE_BUTTON_4_UP,
     .button_id = kButtonIdSide,
     .name = "BTN_SIDE",
     .down_event = EVGET_STRINGIFY(RI_MOUSE_BUTTON_4_DOWN),
     .up_event = EVGET_STRINGIFY(RI_MOUSE_BUTTON_4_UP)},
    {.down_flag = RI_MOUSE_BUTTON_5_DOWN,
     .up_flag = RI_MOUSE_BUTTON_5_UP,
     .button_id = kButtonIdExtra,
     .name = "BTN_EXTRA",
     .down_event = EVGET_STRINGIFY(RI_MOUSE_BUTTON_5_DOWN),
     .up_event = EVGET_STRINGIFY(RI_MOUSE_BUTTON_5_UP)},
}};

std::uint64_t ToMicros(const evget::TimestampType& timestamp) {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(timestamp.time_since_epoch()).count()
    );
}

} // namespace

evgetwindows::EventTransformer::EventTransformer(
    WindowsQueryApi& query,
    HidQueryApi& hid_query,
    ModifierTracker& tracker
)
    : query_{query}, hid_query_{hid_query}, tracker_{tracker} {}

evget::Data evgetwindows::EventTransformer::TransformEvent(evget::InputEvent<RawEvent> event) {
    const auto& raw = event.ViewData();

    // The mouse type's value is zero, so a device change with a default header must not reach the dwType switch.
    if (const auto* change = std::get_if<DeviceChange>(&raw.data)) {
        if (!change->arrival) {
            hid_query_.get().RemoveDevice(raw.header.hDevice);
            RemoveDevice(raw.header.hDevice);
        }

        return evget::Data{};
    }

    std::string device_name = query_.get().DeviceName(raw.header.hDevice).value_or(std::string{kInjectedDeviceName});
    const std::string& device_uuid = device_ids_.UuidDeterministic(std::format("evget:windows:device:{}", device_name));

    auto ctx = EventContext{
        .timestamp = event.GetTimestamp(),
        .device_type = evget::DeviceType::kUnknown,
        .device_name = std::move(device_name),
        .device_uuid = device_uuid,
        .system_event = {},
    };
    auto data = evget::Data{};

    switch (raw.header.dwType) {
        case RIM_TYPEMOUSE:
            ctx.device_type = evget::DeviceType::kMouse;

            if (const auto* mouse = std::get_if<RAWMOUSE>(&raw.data)) {
                BuildMouse(data, ctx, *mouse);
            }

            break;
        case RIM_TYPEKEYBOARD:
            ctx.device_type = evget::DeviceType::kKeyboard;

            if (const auto* keyboard = std::get_if<RAWKEYBOARD>(&raw.data)) {
                BuildKeyboard(data, ctx, *keyboard);
            }

            break;
        case RIM_TYPEHID: {
            if (raw.header.hDevice == nullptr) {
                break;
            }

            ctx.device_type = hid_query_.get().ClassifyDevice(raw.header.hDevice);
            if (ctx.device_type != evget::DeviceType::kTouchscreen && ctx.device_type != evget::DeviceType::kTouchpad) {
                break;
            }

            if (const auto* payload = std::get_if<HidPayload>(&raw.data)) {
                BuildHid(data, ctx, *payload, raw.header.hDevice);
            }

            break;
        }
        default:
            break;
    }

    return data;
}

void evgetwindows::EventTransformer::BuildMouse(evget::Data& data, EventContext& ctx, const RAWMOUSE& mouse) {
    const auto event_time = ToMicros(ctx.timestamp);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
    const USHORT button_flags = mouse.usButtonFlags;
    const USHORT button_data = mouse.usButtonData;
    // NOLINTEND(cppcoreguidelines-pro-type-union-access)

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) != 0) {
        ctx.system_event = EVGET_STRINGIFY(MOUSE_MOVE_ABSOLUTE);
        auto builder = evget::MouseMove{};

        SetBaseFields(builder, ctx, event_time);
        SetRelativeFromAbsolute(builder, ctx.device_uuid.get(), mouse.lLastX, mouse.lLastY);
        builder.Build(data);
    } else if (mouse.lLastX != 0 || mouse.lLastY != 0) {
        ctx.system_event = EVGET_STRINGIFY(MOUSE_MOVE_RELATIVE);
        auto builder = evget::MouseMove{};

        SetBaseFields(builder, ctx, event_time);
        builder.PositionX(mouse.lLastX).PositionY(mouse.lLastY);
        builder.Build(data);
    }

    // NOLINTBEGIN(hicpp-signed-bitwise)
    if ((button_flags & (RI_MOUSE_WHEEL | RI_MOUSE_HWHEEL)) != 0) {
        const bool horizontal = (button_flags & RI_MOUSE_HWHEEL) != 0;
        // NOLINTEND(hicpp-signed-bitwise)
        ctx.system_event = horizontal ? EVGET_STRINGIFY(RI_MOUSE_HWHEEL) : EVGET_STRINGIFY(RI_MOUSE_WHEEL);

        // Raw Input stores the wheel in usButtonData.
        const auto raw_delta = static_cast<SHORT>(button_data);
        const double increment = static_cast<double>(raw_delta) / static_cast<double>(WHEEL_DELTA);

        auto builder = evget::MouseScroll{};
        SetBaseFields(builder, ctx, event_time);
        // Windows is positive for up scroll, which is different to libinput that treats positive as down scroll.
        if (horizontal) {
            builder.Horizontal(increment);
        } else {
            builder.Vertical(-increment);
        }
        builder.Build(data);
    }

    for (const auto& button : kMouseButtons) {
        const std::array<ButtonTransition, 2> transitions{
            {{.flag = button.down_flag, .system_event = button.down_event, .action = evget::ButtonAction::kPress},
             {.flag = button.up_flag, .system_event = button.up_event, .action = evget::ButtonAction::kRelease}}
        };

        for (const auto& transition : transitions) {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            if ((button_flags & transition.flag) == 0) {
                continue;
            }

            ctx.system_event = std::string{transition.system_event};

            auto builder = evget::MouseClick{};
            SetBaseFields(builder, ctx, event_time);
            builder.Button(button.button_id).ButtonName(std::string{button.name}).Action(transition.action);
            builder.Build(data);
        }
    }
}

void evgetwindows::EventTransformer::BuildKeyboard(evget::Data& data, EventContext& ctx, const RAWKEYBOARD& keyboard) {
    if (keyboard.MakeCode == KEYBOARD_OVERRUN_MAKE_CODE || keyboard.VKey >= UCHAR_MAX) {
        return;
    }

    ctx.system_event = EVGET_STRINGIFY(RIM_TYPEKEYBOARD);
    // NOLINTBEGIN(hicpp-signed-bitwise)
    const bool down = (keyboard.Flags & RI_KEY_BREAK) == 0;
    const bool is_extended = (keyboard.Flags & RI_KEY_E0) != 0;
    // NOLINTEND(hicpp-signed-bitwise)
    const UINT resolved_vk = ResolveVk(keyboard);

    auto builder = evget::Key{};
    SetBaseFields(builder, ctx, ToMicros(ctx.timestamp));
    builder.Button(keyboard.VKey).Action(down ? evget::ButtonAction::kPress : evget::ButtonAction::kRelease);

    auto name = VkToKeysymName(resolved_vk, is_extended);
    if (!name.empty()) {
        builder.ButtonName(std::move(name));
    }

    if (down) {
        auto character = query_.get().CharacterFor(resolved_vk, keyboard.MakeCode, tracker_.get().KeyState());
        if (character.has_value()) {
            builder.Character(std::move(*character));
        }
    }

    builder.Build(data);

    tracker_.get().Update(keyboard);
}

void evgetwindows::EventTransformer::BuildHid(
    evget::Data& data,
    EventContext& ctx,
    const HidPayload& payload,
    HANDLE device
) {
    ctx.system_event = EVGET_STRINGIFY(RIM_TYPEHID);

    const auto report =
        hid_query_.get().DecodeReport(device, std::span{payload.report.data(), static_cast<std::size_t>(payload.size)});
    if (!report.has_value()) {
        return;
    }

    auto& state = touch_devices_[device];
    state.device_uuid = ctx.device_uuid.get();
    state.device_name = ctx.device_name;
    state.device_type = ctx.device_type;

    const auto event_time = ToMicros(ctx.timestamp);

    // A defined row order for a multi contact frame.
    std::vector<HidContact> contacts = report->contacts;
    std::ranges::sort(contacts, {}, &HidContact::contact_id);

    for (const auto& contact : contacts) {
        if (!contact.tip_down || !contact.confident || state.tracked.contains(contact.contact_id)) {
            continue;
        }

        const auto touch_id = static_cast<int>(contact.contact_id);

        auto move_builder = evget::MouseMove{};
        SetBaseFields(move_builder, ctx, event_time);
        move_builder.TouchId(touch_id);
        move_builder.Build(data);

        auto click_builder = evget::MouseClick{};
        SetBaseFields(click_builder, ctx, event_time);
        click_builder.Action(evget::ButtonAction::kPress).TouchId(touch_id);
        click_builder.Build(data);

        state.tracked.insert(contact.contact_id);
    }
}

void evgetwindows::EventTransformer::RemoveDevice(HANDLE device) {
    touch_devices_.erase(device);
}

void evgetwindows::EventTransformer::SetRelativeFromAbsolute(
    evget::MouseMove& builder,
    const std::string& device_uuid,
    LONG abs_x,
    LONG abs_y
) {
    if (previous_absolute_x_.contains(device_uuid) && previous_absolute_y_.contains(device_uuid)) {
        builder.PositionX(abs_x - previous_absolute_x_[device_uuid])
            .PositionY(abs_y - previous_absolute_y_[device_uuid]);
    }

    previous_absolute_x_[device_uuid] = abs_x;
    previous_absolute_y_[device_uuid] = abs_y;
}
