#ifndef EVGETX11_XEVENTSWITCHTOUCH_H
#define EVGETX11_XEVENTSWITCHTOUCH_H

#include <X11/X.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <evget/event/button_action.h>
#include <evget/event/data.h>
#include <evget/event/device_type.h>

#include <chrono>
#include <optional>
#include <string>

#include "evget/error.h"
#include "evgetx11/event_switch.h"
#include "evgetx11/input_event.h"

namespace evgetx11 {

class EventSwitchTouch {
public:
    explicit EventSwitchTouch() = default;

    bool SwitchOnEvent(
        const InputEvent& event,
        evget::Data& data,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
    void RefreshDevices(
        int device_id,
        std::optional<int> pointer_id,
        evget::DeviceType device,
        const std::string& name,
        const XIDeviceInfo& info,
        evgetx11::EventSwitch& x_event_switch
    );

private:
    static void TouchButton(
        const InputEvent& event,
        evget::Data& data,
        evget::ButtonAction action,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );

    static void TouchMotion(
        const InputEvent& event,
        evget::Data& data,
        evgetx11::EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
};

bool evgetx11::EventSwitchTouch::SwitchOnEvent(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    switch (event.GetEventType()) {
        case XI_RawTouchBegin:
            TouchMotion(event, data, x_event_switch, get_time);
            TouchButton(event, data, evget::ButtonAction::kPress, x_event_switch, get_time);
            return true;
        case XI_RawTouchUpdate:
            TouchMotion(event, data, x_event_switch, get_time);
            return true;
        case XI_RawTouchEnd:
            TouchMotion(event, data, x_event_switch, get_time);
            TouchButton(event, data, evget::ButtonAction::kRelease, x_event_switch, get_time);
            return true;
        default:
            return false;
    }
}

void evgetx11::EventSwitchTouch::TouchButton(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evget::ButtonAction action,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    if (x_event_switch.HasDevice(raw_event.sourceid)) {
        x_event_switch.AddButtonEvent(raw_event, event.GetTimestamp(), data, action, raw_event.detail, get_time);
    }
}

void evgetx11::EventSwitchTouch::TouchMotion(
    const evgetx11::InputEvent& event,
    evget::Data& data,
    evgetx11::EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    if (x_event_switch.HasDevice(raw_event.sourceid)) {
        x_event_switch.AddMotionEvent(raw_event, event.GetTimestamp(), data, get_time);
    }
}
} // namespace evgetx11

#endif
