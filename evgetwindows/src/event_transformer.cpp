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
#include <ranges>
#include <set>
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
constexpr std::string_view kButtonNameLeft{"BTN_LEFT"};
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
     .name = kButtonNameLeft,
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

constexpr std::uint32_t kMaxContactsPerFrame = 64;
constexpr std::size_t kMaxTrackedContacts = 64;

void AccumulateContacts(
    std::vector<evgetwindows::HidContact>& accumulated,
    const std::vector<evgetwindows::HidContact>& contacts
) {
    for (const auto& contact : contacts) {
        if (accumulated.size() >= kMaxContactsPerFrame) {
            return;
        }

        accumulated.push_back(contact);
    }
}

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

    // A device change makes dwType 0, i.e. RIM_TYPEMOUSE, so we should return early here on it.
    if (const auto* change = std::get_if<DeviceChange>(&raw.data)) {
        auto removal = evget::Data{};
        if (!change->arrival) {
            RemoveDevice(removal, raw.header.hDevice, event.GetTimestamp());
        }

        return removal;
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
    const bool touchscreen = ctx.device_type == evget::DeviceType::kTouchscreen;

    // A touchscreen is mapped to one display, whereas a touchpad goes onto any display the pointer is on.
    if (touchscreen && !state.monitor.has_value()) {
        state.monitor = query_.get().MappedMonitor(device);
    }
    const auto monitor = touchscreen ? state.monitor : query_.get().PointerMonitor();
    const auto range = hid_query_.get().AxisRange(device);

    bool complete = false;
    if (report->contact_count.has_value()) {
        const auto count = *report->contact_count;
        if (count > 0) {
            // A frame that never completed is a cancelled one.
            if (state.frame_open) {
                ReleaseTrackedContacts(data, ctx, state, event_time);
            }

            state.frame_open = true;
            state.expected = std::min(count, kMaxContactsPerFrame);
            state.accumulated.clear();
            AccumulateContacts(state.accumulated, report->contacts);
            complete = state.accumulated.size() >= state.expected;
        } else if (state.frame_open) {
            AccumulateContacts(state.accumulated, report->contacts);
            complete = state.accumulated.size() >= state.expected;
        }
    } else if (!report->contacts.empty()) {
        // A descriptor omitting the contact count still completes.
        state.frame_open = true;
        state.expected = 0;
        state.accumulated.clear();
        AccumulateContacts(state.accumulated, report->contacts);
        complete = true;
    }

    if (complete) {
        BuildTouchFrame(data, ctx, state, range, monitor, event_time);
    }

    BuildPadButton(data, ctx, state, report->button_one_down, event_time);
}

void evgetwindows::EventTransformer::BuildTouchFrame(
    evget::Data& data,
    EventContext& ctx,
    TouchDeviceState& state,
    const std::optional<HidAxisRange>& range,
    const std::optional<MonitorInfo>& monitor,
    std::uint64_t event_time
) {
    auto contacts = std::move(state.accumulated);
    state.accumulated.clear();
    state.frame_open = false;
    state.expected = 0;

    // A defined row order for a multi contact frame, and one row set for a contact the frame repeated.
    std::ranges::stable_sort(contacts, {}, &HidContact::contact_id);
    // Keep only the last sample when there is a repeat.
    const auto duplicates = std::ranges::unique(std::views::reverse(contacts), {}, &HidContact::contact_id);
    contacts.erase(contacts.begin(), duplicates.begin().base());

    std::set<std::uint32_t> present;
    for (const auto& contact : contacts) {
        present.insert(contact.contact_id);
        BuildTouchContact(data, ctx, state, contact, range, monitor, event_time);
    }

    std::erase_if(state.rejected, [&present](const auto contact_id) { return !present.contains(contact_id); });

    // When the confidence is set it means the device considers the contact intentional.
    const auto stale = state.tracked;
    for (const auto contact_id : stale) {
        if (present.contains(contact_id)) {
            continue;
        }

        BuildTouchRelease(data, ctx, state, contact_id, event_time);
        state.tracked.erase(contact_id);
    }
}

void evgetwindows::EventTransformer::BuildTouchContact(
    evget::Data& data,
    EventContext& ctx,
    TouchDeviceState& state,
    const HidContact& contact,
    const std::optional<HidAxisRange>& range,
    const std::optional<MonitorInfo>& monitor,
    std::uint64_t event_time
) {
    // A set confidence bit means the device considers the contact intentional.
    const bool active = contact.tip_down && contact.confident;
    if (state.tracked.contains(contact.contact_id)) {
        if (active) {
            BuildTouchContactDown(data, ctx, state, contact, range, monitor, event_time);
            return;
        }

        BuildTouchRelease(data, ctx, state, contact.contact_id, event_time);
        state.tracked.erase(contact.contact_id);
        // If the tip is down, then state should be rejected if confidence is back.
        if (contact.tip_down) {
            state.rejected.insert(contact.contact_id);
        }
        return;
    }

    if (!contact.tip_down) {
        state.rejected.erase(contact.contact_id);
        return;
    }

    if (!active) {
        state.rejected.insert(contact.contact_id);
        return;
    }

    // The device rejected this contact while it was down, so a returning confidence must not press it.
    if (state.rejected.contains(contact.contact_id)) {
        return;
    }

    if (state.tracked.size() >= kMaxTrackedContacts) {
        ReleaseTrackedContacts(data, ctx, state, event_time);
    }

    state.tracked.insert(contact.contact_id);
    BuildTouchContactDown(data, ctx, state, contact, range, monitor, event_time);

    auto click_builder = evget::MouseClick{};
    SetBaseFields(click_builder, ctx, event_time);
    if (state.device_type == evget::DeviceType::kTouchscreen && monitor.has_value()) {
        click_builder.Screen(monitor->name);
    }
    click_builder.Action(evget::ButtonAction::kPress).TouchId(static_cast<int>(contact.contact_id));
    click_builder.Build(data);
}

void evgetwindows::EventTransformer::BuildTouchContactDown(
    evget::Data& data,
    EventContext& ctx,
    const TouchDeviceState& state,
    const HidContact& contact,
    const std::optional<HidAxisRange>& range,
    const std::optional<MonitorInfo>& monitor,
    std::uint64_t event_time
) {
    // The first sample of a contact has no position, matching the libinput touch row shape.
    auto move_builder = evget::MouseMove{};
    SetBaseFields(move_builder, ctx, event_time);
    if (state.device_type == evget::DeviceType::kTouchscreen && monitor.has_value()) {
        move_builder.Screen(monitor->name);
    }
    move_builder.TouchId(static_cast<int>(contact.contact_id));
    if (monitor.has_value() && range.has_value()) {
        SetTouchRelativePosition(move_builder, state.device_uuid, contact, *range, *monitor);
    }
    move_builder.Build(data);
}

void evgetwindows::EventTransformer::BuildTouchRelease(
    evget::Data& data,
    EventContext& ctx,
    const TouchDeviceState& state,
    std::uint32_t contact_id,
    std::uint64_t event_time
) {
    const auto touch_id = static_cast<int>(contact_id);
    const bool screen_named = state.device_type == evget::DeviceType::kTouchscreen && state.monitor.has_value();

    // Windows reports the last known position with the tip switch clear, but the release row stays positionless.
    auto move_builder = evget::MouseMove{};
    SetBaseFields(move_builder, ctx, event_time);
    if (screen_named) {
        move_builder.Screen(state.monitor->name);
    }
    move_builder.TouchId(touch_id);
    move_builder.Build(data);

    auto click_builder = evget::MouseClick{};
    SetBaseFields(click_builder, ctx, event_time);
    if (screen_named) {
        click_builder.Screen(state.monitor->name);
    }
    click_builder.Action(evget::ButtonAction::kRelease).TouchId(touch_id);

    ClearTouchPosition(state.device_uuid, contact_id);

    click_builder.Build(data);
}

void evgetwindows::EventTransformer::ReleaseTrackedContacts(
    evget::Data& data,
    EventContext& ctx,
    TouchDeviceState& state,
    std::uint64_t event_time
) {
    for (const auto contact_id : state.tracked) {
        BuildTouchRelease(data, ctx, state, contact_id, event_time);
    }

    state.tracked.clear();
}

void evgetwindows::EventTransformer::BuildPadButton(
    evget::Data& data,
    EventContext& ctx,
    TouchDeviceState& state,
    bool button_one_down,
    std::uint64_t event_time
) {
    if (state.device_type != evget::DeviceType::kTouchpad || button_one_down == state.button_one_down) {
        return;
    }

    state.button_one_down = button_one_down;

    auto builder = evget::MouseClick{};
    SetBaseFields(builder, ctx, event_time);
    builder.Button(kButtonIdLeft)
        .ButtonName(std::string{kButtonNameLeft})
        .Action(button_one_down ? evget::ButtonAction::kPress : evget::ButtonAction::kRelease);
    builder.Build(data);
}

void evgetwindows::EventTransformer::RemoveDevice(
    evget::Data& data,
    HANDLE device,
    const evget::TimestampType& timestamp
) {
    const auto entry = touch_devices_.find(device);
    if (entry != touch_devices_.end()) {
        auto& state = entry->second;

        // Build this from the remembered state as a removed device's name cannot be resolved.
        auto ctx = EventContext{
            .timestamp = timestamp,
            .device_type = state.device_type,
            .device_name = state.device_name,
            .device_uuid = state.device_uuid,
            .system_event = EVGET_STRINGIFY(RIM_TYPEHID),
        };

        ReleaseTrackedContacts(data, ctx, state, ToMicros(timestamp));
        touch_devices_.erase(entry);
    }

    hid_query_.get().RemoveDevice(device);
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

void evgetwindows::EventTransformer::ClearTouchPosition(const std::string& device_uuid, std::uint32_t contact_id) {
    const auto key = std::make_pair(device_uuid, contact_id);
    previous_touch_x_.erase(key);
    previous_touch_y_.erase(key);
}

void evgetwindows::EventTransformer::SetTouchRelativePosition(
    evget::MouseMove& builder,
    const std::string& device_uuid,
    const HidContact& contact,
    const HidAxisRange& range,
    const MonitorInfo& monitor
) {
    if (!contact.position_x.has_value() || !contact.position_y.has_value()) {
        return;
    }

    const auto span_x = static_cast<double>(range.max_x) - static_cast<double>(range.min_x);
    const auto span_y = static_cast<double>(range.max_y) - static_cast<double>(range.min_y);
    if (span_x <= 0.0 || span_y <= 0.0) {
        return;
    }

    const double pixel_x =
        (static_cast<double>(*contact.position_x) - static_cast<double>(range.min_x)) / span_x * monitor.width;
    const double pixel_y =
        (static_cast<double>(*contact.position_y) - static_cast<double>(range.min_y)) / span_y * monitor.height;

    // Each contact carries its own stream, parallel contacts do not share a sample.
    const auto key = std::make_pair(device_uuid, contact.contact_id);
    if (previous_touch_x_.contains(key) && previous_touch_y_.contains(key)) {
        builder.PositionX(pixel_x - previous_touch_x_[key]).PositionY(pixel_y - previous_touch_y_[key]);
    }

    previous_touch_x_[key] = pixel_x;
    previous_touch_y_[key] = pixel_y;
}
