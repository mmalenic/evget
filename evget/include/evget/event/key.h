/**
 * \file key.h
 * \brief Keyboard event builder for creating key events.
 */

#ifndef EVGET_EVENT_KEY_H
#define EVGET_EVENT_KEY_H

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
 * \brief Represents a key event.
 */
class Key {
public:
    /**
     * \brief Add an interval in microseconds.
     * \param interval time interval in microseconds
     * \return reference to this `Key` object
     */
    Key& Interval(IntervalType interval);

    /**
     * \brief Add an interval in microseconds.
     * \param interval optional time interval in microseconds
     * \return reference to this `Key` object
     */
    Key& Interval(std::optional<IntervalType> interval);

    /**
     * \brief Add a date timestamp.
     * \param timestamp timestamp of the event
     * \return reference to this `Key` object
     */
    Key& Timestamp(TimestampType timestamp);

    /**
     * \brief Key device.
     * \param device type of the keyboard device
     * \return reference to this `Key` object
     */
    Key& Device(DeviceType device);

    /**
     * \brief Add position x.
     * \param x_pos x coordinate position
     * \return reference to this `Key` object
     */
    Key& PositionX(double x_pos);

    /**
     * \brief Add position y.
     * \param y_pos y coordinate position
     * \return reference to this `Key` object
     */
    Key& PositionY(double y_pos);

    /**
     * \brief Add action.
     * \param action key action
     * \return reference to this `Key` object
     */
    Key& Action(ButtonAction action);

    /**
     * \brief Add id.
     * \param button key identifier
     * \return reference to this `Key` object
     */
    Key& Button(int button);

    /**
     * \brief Add id.
     * \param name key name
     * \return reference to this `Key` object
     */
    Key& Name(std::string name);

    /**
     * \brief Add character.
     * \param character character representation of the key
     * \return reference to this `Key` object
     */
    Key& Character(std::string character);

    /**
     * \brief Add the device name.
     * \param device_name name of the input device
     * \return reference to this `Key` object
     */
    Key& DeviceName(std::string device_name);

    /**
     * \brief Add the focus window name.
     * \param name name of the focused window
     * \return reference to this `Key` object
     */
    Key& FocusWindowName(std::string name);

    /**
     * \brief Add the focus window position x.
     * \param x_pos x coordinate of the focused window
     * \return reference to this `Key` object
     */
    Key& FocusWindowPositionX(double x_pos);

    /**
     * \brief Add the focus window position y.
     * \param y_pos y coordinate of the focused window
     * \return reference to this `Key` object
     */
    Key& FocusWindowPositionY(double y_pos);

    /**
     * \brief Add the focus window width.
     * \param width width of the focused window
     * \return reference to this `Key` object
     */
    Key& FocusWindowWidth(double width);

    /**
     * \brief Add the focus window height.
     * \param height height of the focused window
     * \return reference to this `Key` object
     */
    Key& FocusWindowHeight(double height);

    /**
     * \brief Add the screen number.
     * \param screen screen number identifier
     * \return reference to this `Key` object
     */
    Key& Screen(int screen);

    /**
     * \brief Add modifier value.
     * \param modifier modifier state
     * \return reference to this `Key` object
     */
    Key& Modifier(ModifierValue modifier);

    /**
     * \brief Build key event.
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
    std::optional<std::string> character_;
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
