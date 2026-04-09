/**
 * \file event_switch.h
 * \brief Main event processor for routing X11 input events to appropriate handlers.
 */

#ifndef EVGETX11_EVENT_SWITCH_H
#define EVGETX11_EVENT_SWITCH_H

#include <X11/X.h>
#include <X11/extensions/XInput2.h>
#include <evget/device_id.h>
#include <evget/error.h>
#include <evget/event/button_action.h>
#include <evget/event/data.h>
#include <evget/event/device_type.h>
#include <evget/event/modifier_value.h>
#include <evget/event/schema.h>
#include <evgetx11/x11.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "evget/event/concepts.h"
#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"

namespace evgetx11 {

/// \brief Event source for the X11 backend.
constexpr std::string_view kEventSourceName{"x11"};

/**
 * \brief Handles processing different types of X11 input events and converting them to the
 *        evget data format.
 */
class EventSwitch {
public:
    /**
     * \brief Construct an EventSwitch with an X11 API wrapper.
     * \param x_wrapper reference to the X11 API wrapper
     */
    explicit EventSwitch(X11Api& x_wrapper);

    /**
     * \brief Refresh device information for a specific device.
     * \param device_id the ID of the device
     * \param pointer_id optional pointer device ID
     * \param device device type
     * \param name device name
     * \param info the device information structure
     */
    void RefreshDevices(
        int device_id,
        std::optional<int> pointer_id,
        evget::DeviceType device,
        const std::string& name,
        const XIDeviceInfo& info
    );

    /**
     * \brief Add a button event to the data structure.
     * \param event the raw XI event
     * \param date_time timestamp of the event
     * \param data data structure to add the event to
     * \param action button action
     * \param button button identifier
     * \param get_time function to get time intervals
     */
    void AddButtonEvent(
        const XIRawEvent& event,
        evget::TimestampType date_time,
        evget::Data& data,
        evget::ButtonAction action,
        int button,
        const char* system_event,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );

    /**
     * \brief Add a motion event to the data structure.
     * \param event the raw XI event
     * \param date_time timestamp of the event
     * \param data data structure to add the event to
     * \param system_event stringified event type name
     * \param get_time function to get time intervals
     */
    void AddMotionEvent(
        const XIRawEvent& event,
        evget::TimestampType date_time,
        evget::Data& data,
        const char* system_event,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );

    /**
     * \brief Get the name of a button for a specific device.
     * \param device_id the ID of the device
     * \param button button identifier
     * \return reference to the button name string
     */
    [[nodiscard]] const std::string& GetButtonName(int device_id, int button) const;

    /**
     * \brief Get the device type for the given device ID and event type.
     * \param device_id the ID of the device
     * \param xi2_event_type the XI2 event type
     * \return device type enumeration value
     */
    [[nodiscard]] evget::DeviceType GetDevice(int device_id, int xi2_event_type) const;

    /**
     * \brief Check whether the device with the given ID has already been
     *        encountered in processing.
     * \param device_id the ID of the device to check
     * \return true if device exists, false otherwise
     */
    [[nodiscard]] bool HasDevice(int device_id) const;

    /**
     * \brief Get the UUID for the given device ID.
     * \param device_id the ID of the device
     * \return reference to the UUID
     */
    const std::string& GetDeviceUuid(int device_id);

    /**
     * \brief Set the modifier state for a builder.
     * \tparam T builder type that supports modifier functions
     * \param modifier_state the X11 modifier state bitmask
     * \param builder reference to the builder to modify
     * \return reference to the modified builder
     */
    template <evget::BuilderHasModifier T>
    static T& SetModifierValue(unsigned int modifier_state, T& builder);

    /**
     * \brief Set the window fields for a builder.
     * \tparam T builder type that supports window functions
     * \param builder reference to the builder to modify
     * \return reference to the modified builder
     */
    template <evget::BuilderHasWindowFunctions T>
    T& SetWindowFields(T& builder);

    /**
     * \brief Set the device name fields for a builder.
     * \tparam T builder type that supports device name functions
     * \param builder reference to the builder to modify
     * \param event the raw XI event
     * \param screen screen number
     * \return reference to the modified builder
     */
    template <typename T>
        requires evget::BuilderHasScreenFunction<T> && evget::BuilderHasDeviceName<T>
    T& SetDeviceNameFields(T& builder, const XIRawEvent& event, int screen);

    /**
     * \brief Query the current pointer position for the tracked pointer device.
     * \return query pointer result containing position and modifier state
     */
    QueryPointerResult QueryPointerForDevice();

    /**
     * \brief Set the button map for a device.
     * \param button_info the XI button class information
     * \param device_id the ID of the device
     */
    void SetButtonMap(const XIButtonClassInfo& button_info, int device_id);

private:
    std::reference_wrapper<X11Api> x_wrapper_;
    std::unordered_map<int, std::unordered_map<int, std::string>> button_map_;
    std::unordered_map<int, evget::DeviceType> devices_;
    std::unordered_map<int, std::string> id_to_name_;
    evget::DeviceId<int> device_ids_;
    int pointer_id_{};
};

template <evget::BuilderHasModifier T>
T& EventSwitch::SetModifierValue(unsigned int modifier_state, T& builder) {
    // Based on https://github.com/glfw/glfw/blob/dd8a678a66f1967372e5a5e3deac41ebf65ee127/src/x11_window.c#L215-L235
    if (modifier_state & ShiftMask) {
        builder.Modifier(evget::ModifierValue::kShift);
    }
    if (modifier_state & LockMask) {
        builder.Modifier(evget::ModifierValue::kCapsLock);
    }
    if (modifier_state & ControlMask) {
        builder.Modifier(evget::ModifierValue::kControl);
    }
    if (modifier_state & Mod1Mask) {
        builder.Modifier(evget::ModifierValue::kAlt);
    }
    if (modifier_state & Mod2Mask) {
        builder.Modifier(evget::ModifierValue::kNumLock);
    }
    if (modifier_state & Mod3Mask) {
        builder.Modifier(evget::ModifierValue::kMod3);
    }
    if (modifier_state & Mod4Mask) {
        builder.Modifier(evget::ModifierValue::kSuper);
    }
    if (modifier_state & Mod5Mask) {
        builder.Modifier(evget::ModifierValue::kMod5);
    }

    return builder;
}

template <evget::BuilderHasWindowFunctions T>
T& EventSwitch::SetWindowFields(T& builder) {
    auto window = x_wrapper_.get().GetActiveWindow();

    if (!window.has_value()) {
        spdlog::warn("failed to get active window, falling back on focus window");
        window = x_wrapper_.get().GetFocusWindow();
    }

    if (!window.has_value()) {
        spdlog::warn("failed to get any focus window");
        return builder;
    }

    auto window_name = x_wrapper_.get().GetWindowName(*window);
    auto window_position = x_wrapper_.get().GetWindowPosition(*window);
    auto window_size = x_wrapper_.get().GetWindowSize(*window);

    if (window_name.has_value()) {
        builder.FocusWindowName(*window_name);
    }

    if (window_position.has_value()) {
        builder.FocusWindowPositionX(window_position->width);
        builder.FocusWindowPositionY(window_position->height);
    }

    if (window_size.has_value()) {
        builder.FocusWindowWidth(window_size->width);
        builder.FocusWindowHeight(window_size->height);
    }

    return builder;
}

template <typename T>
    requires evget::BuilderHasScreenFunction<T> && evget::BuilderHasDeviceName<T>
T& EventSwitch::SetDeviceNameFields(T& builder, const XIRawEvent& event, int screen) {
    auto name = id_to_name_.at(event.sourceid);

    return builder.DeviceName(name).Screen(screen);
}

void EventSwitch::AddMotionEvent(
    const XIRawEvent& event,
    evget::TimestampType date_time,
    evget::Data& data,
    const char* system_event,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto query_pointer = this->x_wrapper_.get().QueryPointer(pointer_id_);

    evget::MouseMove builder{};
    builder.Interval(get_time(event.time))
        .Timestamp(date_time)
        .Device(GetDevice(event.sourceid, event.evtype))
        .DeviceId(GetDeviceUuid(event.sourceid))
        .SystemEvent(system_event)
        .PositionX(query_pointer.root_x)
        .PositionY(query_pointer.root_y)
        .EventSource(std::string{kEventSourceName});

    SetModifierValue(query_pointer.modifier_state.effective, builder);
    SetWindowFields(builder);

    SetDeviceNameFields(builder, event, query_pointer.screen_number);

    builder.Build(data);
}

void EventSwitch::AddButtonEvent(
    const XIRawEvent& event,
    evget::TimestampType date_time,
    evget::Data& data,
    evget::ButtonAction action,
    int button,
    const char* system_event,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto query_pointer = this->x_wrapper_.get().QueryPointer(pointer_id_);

    evget::MouseClick builder{};
    builder.Interval(get_time(event.time))
        .Timestamp(date_time)
        .Device(GetDevice(event.sourceid, event.evtype))
        .DeviceId(GetDeviceUuid(event.sourceid))
        .SystemEvent(system_event)
        .PositionX(query_pointer.root_x)
        .PositionY(query_pointer.root_y)
        .Action(action)
        .Button(button)
        .ButtonName(button_map_[event.sourceid][button])
        .EventSource(std::string{kEventSourceName});
    SetModifierValue(query_pointer.modifier_state.effective, builder);
    SetWindowFields(builder);

    SetDeviceNameFields(builder, event, query_pointer.screen_number);

    builder.Build(data);
}
} // namespace evgetx11

#endif
