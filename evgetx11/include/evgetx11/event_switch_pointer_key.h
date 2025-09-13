#ifndef EVGETX11_EVENT_SWITCH_POINTER_KEY_H
#define EVGETX11_EVENT_SWITCH_POINTER_KEY_H

#include <X11/X.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <evget/error.h>
#include <evget/event/button_action.h>
#include <evget/event/data.h>
#include <evget/event/device_type.h>
#include <xorg/xserver-properties.h>

#include <chrono>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>

#include "evget/event/key.h"
#include "evget/event/mouse_scroll.h"
#include "evgetx11/event_switch.h"
#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

namespace evgetx11 {

class EventSwitchPointerKey {
public:
    explicit EventSwitchPointerKey(X11Api& x_wrapper);

    void RefreshDevices(
        int device_id,
        std::optional<int> pointer_id,
        evget::DeviceType device,
        const std::string& name,
        const XIDeviceInfo& info,
        evgetx11::EventSwitch& x_event_switch
    );
    bool SwitchOnEvent(
        const InputEvent& event,
        evget::Data& data,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );

private:
    static std::map<int, int> GetValuators(const XIValuatorState& valuator_state);

    static void ButtonEvent(
        const InputEvent& event,
        evget::Data& data,
        evget::ButtonAction action,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
    void KeyEvent(
        const InputEvent& event,
        evget::Data& data,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
    void MotionEvent(
        const InputEvent& event,
        evget::Data& data,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
    void ScrollEvent(
        const InputEvent& event,
        evget::Data& data,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );

    std::reference_wrapper<X11Api> x_wrapper_;

    std::unordered_map<int, std::unordered_map<int, XIScrollClassInfo>> scroll_map_;
    std::unordered_map<int, std::optional<int>> valuator_x_;
    std::unordered_map<int, std::optional<int>> valuator_y_;

    int pointer_id_{};
};

bool evgetx11::EventSwitchPointerKey::SwitchOnEvent(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    switch (event.GetEventType()) {
        case XI_RawMotion:
            MotionEvent(event, data, x_event_switch, get_time);
            ScrollEvent(event, data, x_event_switch, get_time);
            return true;
        case XI_RawButtonPress:
            ButtonEvent(event, data, evget::ButtonAction::kPress, x_event_switch, get_time);
            return true;
        case XI_RawButtonRelease:
            ButtonEvent(event, data, evget::ButtonAction::kRelease, x_event_switch, get_time);
            return true;
        case XI_RawKeyPress:
        case XI_RawKeyRelease:
            KeyEvent(event, data, x_event_switch, get_time);
            return true;
        default:
            return false;
    }
}

void evgetx11::EventSwitchPointerKey::ButtonEvent(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evget::ButtonAction action,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    auto button = x_event_switch.GetButtonName(raw_event.sourceid, raw_event.detail);
    // NOLINTBEGIN(hicpp-signed-bitwise)
    if (!x_event_switch.HasDevice(raw_event.sourceid) || (raw_event.flags & XIPointerEmulated) ||
        button == BTN_LABEL_PROP_BTN_WHEEL_UP || button == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        button == BTN_LABEL_PROP_BTN_HWHEEL_LEFT || button == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }
    // NOLINTEND(hicpp-signed-bitwise)

    x_event_switch.AddButtonEvent(raw_event, event.GetTimestamp(), data, action, raw_event.detail, get_time);
}

void evgetx11::EventSwitchPointerKey::KeyEvent(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    if (!x_event_switch.HasDevice(raw_event.sourceid)) {
        return;
    }

    auto query_pointer = this->x_wrapper_.get().QueryPointer(pointer_id_);

    KeySym key_sym = NoSymbol;

    const std::string character = x_wrapper_.get().LookupCharacter(raw_event, query_pointer, key_sym);

    evget::ButtonAction action = evget::ButtonAction::kRelease;
    if (raw_event.evtype != XI_KeyRelease) {
        // NOLINTBEGIN(hicpp-signed-bitwise)
        action = (raw_event.flags & XIKeyRepeat) ? evget::ButtonAction::kRepeat : evget::ButtonAction::kPress;
        // NOLINTEND(hicpp-signed-bitwise)
    }

    const std::string name = X11ApiImpl::KeySymToString(key_sym);

    evget::Key builder{};
    builder.Interval(get_time(raw_event.time))
        .PositionX(query_pointer.root_x)
        .PositionY(query_pointer.root_y)
        .Device(x_event_switch.GetDevice(raw_event.sourceid))
        .Timestamp(event.GetTimestamp())
        .Action(action)
        .Button(raw_event.detail)
        .Character(character)
        .Name(name);

    EventSwitch::SetModifierValue(query_pointer.modifier_state.effective, builder);
    x_event_switch.SetWindowFields(builder);

    x_event_switch.SetDeviceNameFields(builder, raw_event, query_pointer.screen_number);

    builder.Build(data);
}

void evgetx11::EventSwitchPointerKey::ScrollEvent(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    // NOLINTBEGIN(hicpp-signed-bitwise)
    if (!x_event_switch.HasDevice(raw_event.sourceid) || !scroll_map_.contains(raw_event.sourceid) ||
        raw_event.flags & XIPointerEmulated) {
        return;
    }
    // NOLINTEND(hicpp-signed-bitwise)

    evget::MouseScroll builder{};
    auto valuators = GetValuators(raw_event.valuators);
    std::unordered_map<int, XIScrollClassInfo> processed_valuators{};
    for (const auto& [valuator, info] : scroll_map_[raw_event.sourceid]) {
        if (valuators.contains(valuator)) {
            processed_valuators.try_emplace(valuator, info);
        }
    }

    if (processed_valuators.empty()) {
        return;
    }

    for (const auto& [valuator, info] : processed_valuators) {
        if (info.scroll_type == XIScrollTypeHorizontal) {
            builder.Horizontal(valuators[valuator]);
        } else {
            builder.Vertical(valuators[valuator]);
        }
    }

    auto query_pointer = this->x_wrapper_.get().QueryPointer(pointer_id_);
    builder.Interval(get_time(raw_event.time))
        .Timestamp(event.GetTimestamp())
        .Device(x_event_switch.GetDevice(raw_event.sourceid))
        .PositionX(query_pointer.root_x)
        .PositionY(query_pointer.root_y);

    EventSwitch::SetModifierValue(query_pointer.modifier_state.effective, builder);
    x_event_switch.SetWindowFields(builder);

    x_event_switch.SetDeviceNameFields(builder, raw_event, query_pointer.screen_number);

    builder.Build(data);
}

void evgetx11::EventSwitchPointerKey::MotionEvent(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    // NOLINTBEGIN(hicpp-signed-bitwise)
    if (!x_event_switch.HasDevice(raw_event.sourceid) || (raw_event.flags & XIPointerEmulated)) {
        return;
    }
    // NOLINTEND(hicpp-signed-bitwise)

    auto valuators = GetValuators(raw_event.valuators);
    for (const auto& valuator : valuators | std::views::keys) {
        if (valuator == valuator_x_[raw_event.sourceid] || valuator == valuator_y_[raw_event.sourceid]) {
            x_event_switch.AddMotionEvent(raw_event, event.GetTimestamp(), data, get_time);
            break;
        }
    }
}
} // namespace evgetx11

#endif
