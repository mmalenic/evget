#ifndef EVGET_EVENT_MOUSE_SCROLL_H
#define EVGET_EVENT_MOUSE_SCROLL_H

#include <optional>
#include <string>
#include <vector>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

namespace evget {
/**
 * Represents a mouse wheel event.
 */
class MouseScroll {
public:
    /**
     * Add interval in microseconds.
     */
    MouseScroll& Interval(IntervalType interval);

    /**
     * Add interval in microseconds.
     */
    MouseScroll& Interval(std::optional<IntervalType> interval);

    /**
     * Add date timestamp
     */
    MouseScroll& Timestamp(TimestampType timestamp);

    /**
     * Mouse device.
     */
    MouseScroll& Device(DeviceType device);

    /**
     * Add position x.
     */
    MouseScroll& PositionX(double x_pos);

    /**
     * Add position y.
     */
    MouseScroll& PositionY(double y_pos);

    /**
     * Add vertical scroll.
     */
    MouseScroll& Vertical(double amount);

    /**
     * Add horizontal scroll.
     */
    MouseScroll& Horizontal(double amount);

    /**
     * Add the device name.
     */
    MouseScroll& DeviceName(std::string device_name);

    /**
     * Add the focus window name.
     */
    MouseScroll& FocusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    MouseScroll& FocusWindowPositionX(double x_pos);

    /**
     * Add the focus window position y.
     */
    MouseScroll& FocusWindowPositionY(double y_pos);

    /**
     * Add the focus window width.
     */
    MouseScroll& FocusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    MouseScroll& FocusWindowHeight(double height);

    /**
     * Add the screen number.
     */
    MouseScroll& Screen(int screen);

    /**
     * Add modifier value.
     */
    MouseScroll& Modifier(ModifierValue modifier);

    /**
     * Build mouse wheel event.
     */
    Data& Build(Data& data);

private:
    std::optional<IntervalType> interval_;
    std::optional<TimestampType> timestamp_;
    std::optional<DeviceType> device_;
    std::optional<double> position_x_;
    std::optional<double> position_y_;
    std::optional<double> vertical_;
    std::optional<double> horizontal_;
    std::optional<std::string> device_name_;
    std::optional<std::string> focus_window_name_;
    std::optional<double> focus_window_position_x_;
    std::optional<double> focus_window_position_y_;
    std::optional<double> focus_window_width_;
    std::optional<double> focus_window_height_;
    std::optional<int> screen_;

    std::vector<std::string> modifiers_;
};
}  // namespace evget

#endif
