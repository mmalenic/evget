/**
 * \file mouse_move.h
 * \brief Mouse movement event builder for mouse motion events.
 */

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
 * \brief Represents a mouse move event.
 */
class MouseMove {
public:
    /**
     * \brief Add an interval in microseconds.
     * \param interval time interval in microseconds
     * \return reference to this `MouseMove` object
     */
    MouseMove& Interval(IntervalType interval);

    /**
     * \brief Add an interval in microseconds.
     * \param interval optional time interval in microseconds
     * \return reference to this `MouseMove` object
     */
    MouseMove& Interval(std::optional<IntervalType> interval);

    /**
     * \brief Add a date timestamp.
     * \param timestamp timestamp of the event
     * \return reference to this `MouseMove` object
     */
    MouseMove& Timestamp(TimestampType timestamp);

    /**
     * \brief Mouse device.
     * \param device type of the mouse device
     * \return reference to this `MouseMove` object
     */
    MouseMove& Device(DeviceType device);

    /**
     * \brief Add position x.
     * \param x_pos x coordinate position
     * \return reference to this `MouseMove` object
     */
    MouseMove& PositionX(double x_pos);

    /**
     * \brief Add position y.
     * \param y_pos y coordinate position
     * \return reference to this `MouseMove` object
     */
    MouseMove& PositionY(double y_pos);

    /**
     * \brief Add the device name.
     * \param device_name name of the input device
     * \return reference to this `MouseMove` object
     */
    MouseMove& DeviceName(std::string device_name);

    /**
     * \brief Add the focus window name.
     * \param name name of the focused window
     * \return reference to this `MouseMove` object
     */
    MouseMove& FocusWindowName(std::string name);

    /**
     * \brief Add the focus window position x.
     * \param x_pos x coordinate of the focused window
     * \return reference to this `MouseMove` object
     */
    MouseMove& FocusWindowPositionX(double x_pos);

    /**
     * \brief Add the focus window position y.
     * \param y_pos y coordinate of the focused window
     * \return reference to this `MouseMove` object
     */
    MouseMove& FocusWindowPositionY(double y_pos);

    /**
     * \brief Add the focus window width.
     * \param width width of the focused window
     * \return reference to this `MouseMove` object
     */
    MouseMove& FocusWindowWidth(double width);

    /**
     * \brief Add the focus window height.
     * \param height height of the focused window
     * \return reference to this `MouseMove` object
     */
    MouseMove& FocusWindowHeight(double height);

    /**
     * \brief Add the screen number.
     * \param screen screen number identifier
     * \return reference to this `MouseMove` object
     */
    MouseMove& Screen(int screen);

    /**
     * \brief Add modifier value.
     * \param modifier modifier state
     * \return reference to this `MouseMove` object
     */
    MouseMove& Modifier(ModifierValue modifier);

    /**
     * \brief Build mouse move event.
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
