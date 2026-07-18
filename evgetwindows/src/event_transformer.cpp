#include "evgetwindows/event_transformer.h"

#include <windows.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/key.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"
#include "evget/event/mouse_scroll.h"
#include "evget/input_event.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/vk_keysym.h"

namespace {

constexpr int kButtonIdLeft = 0x110;
constexpr int kButtonIdRight = 0x111;
constexpr int kButtonIdMiddle = 0x112;
constexpr int kButtonIdSide = 0x113;
constexpr int kButtonIdExtra = 0x114;

constexpr USHORT kKeyboardOverrun = 0xFF;

struct MouseButton {
    USHORT down_flag;
    USHORT up_flag;
    int button_id;
    std::string_view name;
    std::string_view down_event;
    std::string_view up_event;
};

constexpr std::array<MouseButton, 5> kMouseButtons{{
    {RI_MOUSE_LEFT_BUTTON_DOWN, RI_MOUSE_LEFT_BUTTON_UP, kButtonIdLeft, "BTN_LEFT", "RI_MOUSE_LEFT_BUTTON_DOWN",
     "RI_MOUSE_LEFT_BUTTON_UP"},
    {RI_MOUSE_RIGHT_BUTTON_DOWN, RI_MOUSE_RIGHT_BUTTON_UP, kButtonIdRight, "BTN_RIGHT", "RI_MOUSE_RIGHT_BUTTON_DOWN",
     "RI_MOUSE_RIGHT_BUTTON_UP"},
    {RI_MOUSE_MIDDLE_BUTTON_DOWN, RI_MOUSE_MIDDLE_BUTTON_UP, kButtonIdMiddle, "BTN_MIDDLE",
     "RI_MOUSE_MIDDLE_BUTTON_DOWN", "RI_MOUSE_MIDDLE_BUTTON_UP"},
    {RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP, kButtonIdSide, "BTN_SIDE", "RI_MOUSE_BUTTON_4_DOWN",
     "RI_MOUSE_BUTTON_4_UP"},
    {RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP, kButtonIdExtra, "BTN_EXTRA", "RI_MOUSE_BUTTON_5_DOWN",
     "RI_MOUSE_BUTTON_5_UP"},
}};

} // namespace

evgetwindows::EventTransformer::EventTransformer(WindowsQueryApi& query, ModifierTracker& tracker)
    : query_{query}, tracker_{tracker} {}

std::uint64_t evgetwindows::EventTransformer::ToMicros(const evget::TimestampType& timestamp) {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(timestamp.time_since_epoch()).count()
    );
}

evget::Data evgetwindows::EventTransformer::TransformEvent(evget::InputEvent<RawEvent> event) {
    const auto& raw = event.ViewData();
    const HANDLE device = raw.header.hDevice;

    std::string device_name = device == nullptr
        ? std::string{kInjectedDeviceName}
        : query_.get().DeviceName(device).value_or(std::string{kInjectedDeviceName});
    const std::string& device_uuid = device_ids_.UuidDeterministic(std::format("evget:windows:device:{}", device_name));

    auto device_type = evget::DeviceType::kUnknown;
    switch (raw.header.dwType) {
        case RIM_TYPEMOUSE:
            device_type = evget::DeviceType::kMouse;
            break;
        case RIM_TYPEKEYBOARD:
            device_type = evget::DeviceType::kKeyboard;
            break;
        default:
            break;
    }

    auto ctx = EventContext{
        .timestamp = event.GetTimestamp(),
        .device_type = device_type,
        .device_name = std::move(device_name),
        .device_uuid = device_uuid,
        .system_event = {},
    };

    auto data = evget::Data{};
    switch (raw.header.dwType) {
        case RIM_TYPEMOUSE:
            if (const auto* mouse = std::get_if<RAWMOUSE>(&raw.data)) {
                BuildMouse(data, ctx, *mouse);
            }
            break;
        case RIM_TYPEKEYBOARD:
            if (const auto* keyboard = std::get_if<RAWKEYBOARD>(&raw.data)) {
                BuildKeyboard(data, ctx, *keyboard);
            }
            break;
        default:
            break;
    }

    return data;
}

void evgetwindows::EventTransformer::BuildMouse(evget::Data& data, EventContext& ctx, const RAWMOUSE& mouse) {
    const auto event_time = ToMicros(ctx.timestamp);

    if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) != 0) {
        ctx.system_event = "RAWMOUSE_ABSOLUTE";
        auto builder = evget::MouseMove{};
        SetBaseFields(builder, ctx, event_time);
        SetRelativeFromAbsolute(builder, ctx.device_uuid.get(), mouse.lLastX, mouse.lLastY);
        builder.Build(data);
    } else if (mouse.lLastX != 0 || mouse.lLastY != 0) {
        ctx.system_event = "RAWMOUSE_RELATIVE";
        auto builder = evget::MouseMove{};
        SetBaseFields(builder, ctx, event_time);
        builder.PositionX(static_cast<double>(mouse.lLastX)).PositionY(static_cast<double>(mouse.lLastY));
        builder.Build(data);
    }

    if ((mouse.usButtonFlags & (RI_MOUSE_WHEEL | RI_MOUSE_HWHEEL)) != 0) {
        const bool horizontal = (mouse.usButtonFlags & RI_MOUSE_HWHEEL) != 0;
        ctx.system_event = horizontal ? "RI_MOUSE_HWHEEL" : "RI_MOUSE_WHEEL";

        // Raw Input stores the wheel delta directly in usButtonData, not in its high word.
        const auto raw_delta = static_cast<SHORT>(mouse.usButtonData);
        const double notches = static_cast<double>(raw_delta) / static_cast<double>(WHEEL_DELTA);

        auto builder = evget::MouseScroll{};
        SetBaseFields(builder, ctx, event_time);
        // Windows reports wheel-forward as positive; libinput stores down as positive, so vertical is negated.
        if (horizontal) {
            builder.Horizontal(notches);
        } else {
            builder.Vertical(-notches);
        }
        builder.Build(data);
    }

    for (const auto& button : kMouseButtons) {
        const bool down = (mouse.usButtonFlags & button.down_flag) != 0;
        const bool up = (mouse.usButtonFlags & button.up_flag) != 0;
        if (!down && !up) {
            continue;
        }

        ctx.system_event = std::string{down ? button.down_event : button.up_event};
        auto builder = evget::MouseClick{};
        SetBaseFields(builder, ctx, event_time);
        builder.Button(button.button_id)
            .ButtonName(std::string{button.name})
            .Action(down ? evget::ButtonAction::kPress : evget::ButtonAction::kRelease);
        builder.Build(data);
    }
}

void evgetwindows::EventTransformer::BuildKeyboard(evget::Data& data, EventContext& ctx, const RAWKEYBOARD& keyboard) {
    if (keyboard.VKey == kKeyboardOverrun) {
        return;
    }

    ctx.system_event = "RIM_TYPEKEYBOARD";
    const bool down = (keyboard.Flags & RI_KEY_BREAK) == 0;
    const bool is_extended = (keyboard.Flags & RI_KEY_E0) != 0;
    const UINT resolved_vk = ResolveVk(keyboard);

    auto builder = evget::Key{};
    SetBaseFields(builder, ctx, ToMicros(ctx.timestamp));
    builder.Button(static_cast<int>(keyboard.VKey))
        .Action(down ? evget::ButtonAction::kPress : evget::ButtonAction::kRelease);

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

void evgetwindows::EventTransformer::SetRelativeFromAbsolute(
    evget::MouseMove& builder,
    const std::string& device_uuid,
    LONG abs_x,
    LONG abs_y
) {
    if (previous_absolute_x_.contains(device_uuid) && previous_absolute_y_.contains(device_uuid)) {
        builder.PositionX(static_cast<double>(abs_x - previous_absolute_x_[device_uuid]))
            .PositionY(static_cast<double>(abs_y - previous_absolute_y_[device_uuid]));
    }

    previous_absolute_x_[device_uuid] = abs_x;
    previous_absolute_y_[device_uuid] = abs_y;
}
