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
     * \param interval time interval in microseconds
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Interval(IntervalType interval);

    /**
     * \brief Add an interval in microseconds.
     * \param interval optional time interval in microseconds
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Interval(std::optional<IntervalType> interval);

    /**
     * \brief Add the timestamp.
     * \param timestamp timestamp of the event
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Timestamp(TimestampType timestamp);

    /**
     * \brief Mouse device.
     * \param device type of the mouse device
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Device(DeviceType device);

    /**
     * \brief Add position x.
     * \param x_pos x coordinate position
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& PositionX(double x_pos);

    /**
     * \brief Add position y.
     * \param y_pos y coordinate position
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& PositionY(double y_pos);

    /**
     * \brief Add vertical scroll.
     * \param amount vertical scroll amount
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Vertical(double amount);

    /**
     * \brief Add horizontal scroll.
     * \param amount horizontal scroll amount
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Horizontal(double amount);

    /**
     * \brief Add the device name.
     * \param device_name name of the input device
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& DeviceName(std::string device_name);

    /**
     * \brief Add the focus window name.
     * \param name name of the focused window
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& FocusWindowName(std::string name);

    /**
     * \brief Add the focus window position x.
     * \param x_pos x coordinate of the focused window
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& FocusWindowPositionX(double x_pos);

    /**
     * \brief Add the focus window position y.
     * \param y_pos y coordinate of the focused window
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& FocusWindowPositionY(double y_pos);

    /**
     * \brief Add the focus window width.
     * \param width width of the focused window
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& FocusWindowWidth(double width);

    /**
     * \brief Add the focus window height.
     * \param height height of the focused window
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& FocusWindowHeight(double height);

    /**
     * \brief Add the screen number.
     * \param screen screen number identifier
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Screen(int screen);

    /**
     * \brief Add modifier value.
     * \param modifier modifier state
     * \return reference to this `MouseScroll` object
     */
    MouseScroll& Modifier(ModifierValue modifier);

    /**
     * \brief Build mouse wheel event.
     * \param data data container to add the event to
     * \return reference to the data container
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
