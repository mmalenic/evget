/**
 * \file mouse_click.h
 * \brief Mouse click event builder for creating mouse button events.
 */

#ifndef EVGET_EVENT_MOUSE_CLICK_H
#define EVGET_EVENT_MOUSE_CLICK_H

#include <optional>
#include <string>
#include <vector>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

namespace evget {
/**
 * \brief Represents a mouse-click event.
 */
class MouseClick {
public:
    /**
     * \brief Add an interval in microseconds.
     * \param interval time interval in microseconds
     * \return reference to this `MouseClick` object
     */
    MouseClick& Interval(IntervalType interval);

    /**
     * \brief Add an interval in microseconds.
     * \param interval optional time interval in microseconds
     * \return reference to this `MouseClick` object
     */
    MouseClick& Interval(std::optional<IntervalType> interval);

    /**
     * \brief Add a date timestamp.
     * \param timestamp timestamp of the event
     * \return reference to this `MouseClick` object
     */
    MouseClick& Timestamp(TimestampType timestamp);

    /**
     * \brief Mouse device.
     * \param device type of the mouse device
     * \return reference to this `MouseClick` object
     */
    MouseClick& Device(DeviceType device);

    /**
     * \brief Add position x.
     * \param x_pos x coordinate position
     * \return reference to this `MouseClick` object
     */
    MouseClick& PositionX(double x_pos);

    /**
     * \brief Add position y.
     * \param y_pos y coordinate position
     * \return reference to this `MouseClick` object
     */
    MouseClick& PositionY(double y_pos);

    /**
     * \brief Add action.
     * \param action button action
     * \return reference to this `MouseClick` object
     */
    MouseClick& Action(ButtonAction action);

    /**
     * \brief Add id.
     * \param button button identifier
     * \return reference to this `MouseClick` object
     */
    MouseClick& Button(int button);

    /**
     * \brief Add name.
     * \param name button name
     * \return reference to this `MouseClick` object
     */
    MouseClick& Name(std::string name);

    /**
     * \brief Add the device name.
     * \param device_name name of the input device
     * \return reference to this `MouseClick` object
     */
    MouseClick& DeviceName(std::string device_name);

    /**
     * \brief Add the focus window name.
     * \param name name of the focused window
     * \return reference to this `MouseClick` object
     */
    MouseClick& FocusWindowName(std::string name);

    /**
     * \brief Add the focus window position x.
     * \param x_pos x coordinate of the focused window
     * \return reference to this `MouseClick` object
     */
    MouseClick& FocusWindowPositionX(double x_pos);

    /**
     * \brief Add the focus window position y.
     * \param y_pos y coordinate of the focused window
     * \return reference to this `MouseClick` object
     */
    MouseClick& FocusWindowPositionY(double y_pos);

    /**
     * \brief Add the focus window width.
     * \param width width of the focused window
     * \return reference to this `MouseClick` object
     */
    MouseClick& FocusWindowWidth(double width);

    /**
     * \brief Add the focus window height.
     * \param height height of the focused window
     * \return reference to this `MouseClick` object
     */
    MouseClick& FocusWindowHeight(double height);

    /**
     * \brief Add the screen number.
     * \param screen screen number identifier
     * \return reference to this `MouseClick` object
     */
    MouseClick& Screen(int screen);

    /**
     * \brief Add modifier value.
     * \param modifier modifier state
     * \return reference to this `MouseClick` object
     */
    MouseClick& Modifier(ModifierValue modifier);

    /**
     * \brief Build mouse click event.
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
    std::optional<ButtonAction> action_;
    std::optional<int> button_;
    std::optional<std::string> name_;
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
