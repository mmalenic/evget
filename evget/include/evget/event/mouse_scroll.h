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
 * \brief Represents a mouse wheel event.
 */
class MouseScroll {
public:
    /**
     * \brief Add an interval in microseconds.
     */
    MouseScroll& Interval(IntervalType interval);

    /**
     * \brief Add an interval in microseconds.
     */
    MouseScroll& Interval(std::optional<IntervalType> interval);

    /**
     * \brief Add date timestamp.
     */
    MouseScroll& Timestamp(TimestampType timestamp);

    /**
     * \brief Mouse device.
     */
    MouseScroll& Device(DeviceType device);

    /**
     * \brief Add position x.
     */
    MouseScroll& PositionX(double x_pos);

    /**
     * \brief Add position y.
     */
    MouseScroll& PositionY(double y_pos);

    /**
     * \brief Add vertical scroll.
     */
    MouseScroll& Vertical(double amount);

    /**
     * \brief Add horizontal scroll.
     */
    MouseScroll& Horizontal(double amount);

    /**
     * \brief Add the device name.
     */
    MouseScroll& DeviceName(std::string device_name);

    /**
     * \brief Add the focus window name.
     */
    MouseScroll& FocusWindowName(std::string name);

    /**
     * \brief Add the focus window position x.
     */
    MouseScroll& FocusWindowPositionX(double x_pos);

    /**
     * \brief Add the focus window position y.
     */
    MouseScroll& FocusWindowPositionY(double y_pos);

    /**
     * \brief Add the focus window width.
     */
    MouseScroll& FocusWindowWidth(double width);

    /**
     * \brief Add the focus window height.
     */
    MouseScroll& FocusWindowHeight(double height);

    /**
     * \brief Add the screen number.
     */
    MouseScroll& Screen(int screen);

    /**
     * \brief Add modifier value.
     */
    MouseScroll& Modifier(ModifierValue modifier);

    /**
     * \brief Build mouse wheel event.
     */
    Data& Build(Data& data) const;

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
} // namespace evget

#endif
