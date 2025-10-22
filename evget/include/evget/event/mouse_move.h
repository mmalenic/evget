#ifndef EVGET_EVENT_MOUSE_MOVE_H
#define EVGET_EVENT_MOUSE_MOVE_H

#include <optional>
#include <string>
#include <vector>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

namespace evget {
/**
 * Represents a mouse move event.
 */
class MouseMove {
public:
    /**
     * Add an interval in microseconds.
     */
    MouseMove& Interval(IntervalType interval);

    /**
     * Add an interval in microseconds.
     */
    MouseMove& Interval(std::optional<IntervalType> interval);

    /**
     * Add a date timestamp.
     */
    MouseMove& Timestamp(TimestampType timestamp);

    /**
     * Mouse device.
     */
    MouseMove& Device(DeviceType device);

    /**
     * Add position x.
     */
    MouseMove& PositionX(double x_pos);

    /**
     * Add position y.
     */
    MouseMove& PositionY(double y_pos);

    /**
     * Add the device name.
     */
    MouseMove& DeviceName(std::string device_name);

    /**
     * Add the focus window name.
     */
    MouseMove& FocusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    MouseMove& FocusWindowPositionX(double x_pos);

    /**
     * Add the focus window position y.
     */
    MouseMove& FocusWindowPositionY(double y_pos);

    /**
     * Add the focus window width.
     */
    MouseMove& FocusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    MouseMove& FocusWindowHeight(double height);

    /**
     * Add the screen number.
     */
    MouseMove& Screen(int screen);

    /**
     * Add modifier value.
     */
    MouseMove& Modifier(ModifierValue modifier);

    /**
     * Build mouse move event.
     */
    Data& Build(Data& data) const;

private:
    std::optional<IntervalType> interval_;
    std::optional<TimestampType> timestamp_;
    std::optional<DeviceType> device_;
    std::optional<double> position_x_;
    std::optional<double> position_y_;
    std::optional<std::string> device_name_;
    std::optional<std::string> focus_window_name_;
    std::optional<double> focus_window_position_x_;
    std::optional<double> focus_window_position_y_;
    std::optional<double> focus_window_width_;
    std::optional<double> focus_window_height_;
    std::optional<int> screen_;

    std::vector<std::string> modifiers_;
};
} // namespace evget

#endif
