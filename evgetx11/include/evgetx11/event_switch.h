#ifndef EVGETX11_EVENT_SWITCH_H
#define EVGETX11_EVENT_SWITCH_H

#include <X11/X.h>
#include <X11/extensions/XInput2.h>
#include <evget/error.h>
#include <evget/event/button_action.h>
#include <evget/event/data.h>
#include <evget/event/device_type.h>
#include <evget/event/modifier_value.h>
#include <evget/event/schema.h>
#include <evgetx11/x11_api.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <concepts>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "evget/event/mouse_click.h"
#include "evget/event/mouse_move.h"

namespace evgetx11 {
/**
 * Check whether the template parameter is a builder with a modifier function.
 */
template <typename T>
concept BuilderHasModifier = requires(T builder, evget::ModifierValue modifier_value) {
    { builder.Modifier(modifier_value) } -> std::convertible_to<T>;
};

/**
 * Check whether the template parameter is a builder with focus window functions.
 */
template <typename T>
concept BuilderHasWindowFunctions =
    requires(T builder, std::string name, double x_pos, double y_pos, double width, double height) {
        { builder.FocusWindowName(name) } -> std::convertible_to<T>;
        { builder.FocusWindowPositionX(x_pos) } -> std::convertible_to<T>;
        { builder.FocusWindowPositionY(y_pos) } -> std::convertible_to<T>;
        { builder.FocusWindowWidth(width) } -> std::convertible_to<T>;
        { builder.FocusWindowHeight(height) } -> std::convertible_to<T>;
    };

/**
 * Check whether the template parameter is a builder with a device name function.
 */
template <typename T>
concept BuilderHasDeviceNameFunctions = requires(T builder, std::string device_name, int screen) {
    { builder.DeviceName(device_name) } -> std::convertible_to<T>;
    { builder.Screen(screen) } -> std::convertible_to<T>;
};

class EventSwitch {
public:
    explicit EventSwitch(X11Api& x_wrapper);

    void RefreshDevices(
        int device_id,
        std::optional<int> pointer_id,
        evget::DeviceType device,
        const std::string& name,
        const XIDeviceInfo& info
    );

    void AddButtonEvent(
        const XIRawEvent& event,
        evget::TimestampType date_time,
        evget::Data& data,
        evget::ButtonAction action,
        int button,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );
    void AddMotionEvent(
        const XIRawEvent& event,
        evget::TimestampType date_time,
        evget::Data& data,
        evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
    );

    [[nodiscard]] const std::string& GetButtonName(int device_id, int button) const;

    /**
     * Get the device with the given id.
     */
    [[nodiscard]] evget::DeviceType GetDevice(int device_id) const;

    /**
     * Check whether the device with the given id is present.
     */
    [[nodiscard]] bool HasDevice(int device_id) const;

    /**
     * Set the modifier state for a builder.
     */
    template <BuilderHasModifier T>
    static T& SetModifierValue(unsigned int modifier_state, T& builder);

    /**
     * Set the window fields for a builder.
     */
    template <BuilderHasWindowFunctions T>
    T& SetWindowFields(T& builder);

    /**
     * Set the device name fields for a builder.
     */
    template <BuilderHasDeviceNameFunctions T>
    T& SetDeviceNameFields(T& builder, const XIRawEvent& event, int screen);

    /**
     * Set the button map for a device.
     * @param buttonInfo button info
     * @param device_id device id
     */
    void SetButtonMap(const XIButtonClassInfo& button_info, int device_id);

private:
    std::reference_wrapper<X11Api> x_wrapper_;
    std::unordered_map<int, std::unordered_map<int, std::string>> button_map_;
    std::unordered_map<int, evget::DeviceType> devices_;
    std::unordered_map<int, std::string> id_to_name_;
    int pointer_id_{};
};

template <BuilderHasModifier T>
T& evgetx11::EventSwitch::SetModifierValue(unsigned int modifier_state, T& builder) {
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

template <BuilderHasWindowFunctions T>
T& evgetx11::EventSwitch::SetWindowFields(T& builder) {
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

template <BuilderHasDeviceNameFunctions T>
T& evgetx11::EventSwitch::SetDeviceNameFields(T& builder, const XIRawEvent& event, int screen) {
    auto name = id_to_name_.at(event.sourceid);

    return builder.DeviceName(name).Screen(screen);
}

void evgetx11::EventSwitch::AddMotionEvent(
    const XIRawEvent& event,
    evget::TimestampType date_time,
    evget::Data& data,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto query_pointer = this->x_wrapper_.get().QueryPointer(pointer_id_);

    evget::MouseMove builder{};
    builder.Interval(get_time(event.time))
        .Timestamp(date_time)
        .Device(GetDevice(event.sourceid))
        .PositionX(query_pointer.root_x)
        .PositionY(query_pointer.root_y);

    EventSwitch::SetModifierValue(query_pointer.modifier_state.effective, builder);
    SetWindowFields(builder);

    SetDeviceNameFields(builder, event, query_pointer.screen_number);

    builder.Build(data);
}

void evgetx11::EventSwitch::AddButtonEvent(
    const XIRawEvent& event,
    evget::TimestampType date_time,
    evget::Data& data,
    evget::ButtonAction action,
    int button,
    evget::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& get_time
) {
    auto query_pointer = this->x_wrapper_.get().QueryPointer(pointer_id_);

    evget::MouseClick builder{};
    builder.Interval(get_time(event.time))
        .Timestamp(date_time)
        .Device(GetDevice(event.sourceid))
        .PositionX(query_pointer.root_x)
        .PositionY(query_pointer.root_y)
        .Action(action)
        .Button(button)
        .Name(button_map_[event.sourceid][button]);
    EventSwitch::SetModifierValue(query_pointer.modifier_state.effective, builder);
    SetWindowFields(builder);

    SetDeviceNameFields(builder, event, query_pointer.screen_number);

    builder.Build(data);
}
} // namespace evgetx11

#endif
