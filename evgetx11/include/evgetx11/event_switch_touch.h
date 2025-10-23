/**
 * \file event_switch_touch.h
 * \brief Event processor for X11 touch input events.
 */

#ifndef EVGETX11_EVENT_SWITCH_TOUCH_H
#define EVGETX11_EVENT_SWITCH_TOUCH_H

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

/**
 * \brief Event processing for touch events from X11.
 */
class EventSwitchTouch {
public:
    /**
     * \brief Default constructor for `EventSwitchTouch`.
     */
    EventSwitchTouch() = default;

    /**
     * \brief Process a touch input event and convert it to evget data if applicable.
     * \param event the input event to process
     * \param data data structure to add converted events to
     * \param x_event_switch reference to the main event switch
     * \param get_time function to get time intervals
     * \return true if the event was processed, false otherwise
     */
    bool SwitchOnEvent(
        const InputEvent& event,
        evget::Data& data,
        EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
    
    /**
     * \brief Refresh device information for touch devices.
     * \param device_id the ID of the device
     * \param pointer_id optional pointer ID
     * \param device device type
     * \param name device name
     * \param info the XI device information structure
     * \param x_event_switch reference to the main event switch
     */
    void RefreshDevices(
        int device_id,
        std::optional<int> pointer_id,
        evget::DeviceType device,
        const std::string& name,
        const XIDeviceInfo& info,
        EventSwitch& x_event_switch
    );

private:
    static void TouchButton(
        const InputEvent& event,
        evget::Data& data,
        evget::ButtonAction action,
        EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );

    static void TouchMotion(
        const InputEvent& event,
        evget::Data& data,
        EventSwitch& x_event_switch,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
};

bool EventSwitchTouch::SwitchOnEvent(
    const InputEvent& event,
    evget::Data& data,
    EventSwitch& x_event_switch,
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

void EventSwitchTouch::TouchButton(
    const InputEvent& event,
    evget::Data& data,
    evget::ButtonAction action,
    EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    if (x_event_switch.HasDevice(raw_event.sourceid)) {
        x_event_switch.AddButtonEvent(raw_event, event.GetTimestamp(), data, action, raw_event.detail, get_time);
    }
}

void EventSwitchTouch::TouchMotion(
    const InputEvent& event,
    evget::Data& data,
    EventSwitch& x_event_switch,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto raw_event = event.ViewData<XIRawEvent>();
    if (x_event_switch.HasDevice(raw_event.sourceid)) {
        x_event_switch.AddMotionEvent(raw_event, event.GetTimestamp(), data, get_time);
    }
}
} // namespace evgetx11

#endif
