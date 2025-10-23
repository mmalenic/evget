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
     */
    Key& Interval(IntervalType interval);

    /**
     * \brief Add an interval in microseconds.
     */
    Key& Interval(std::optional<IntervalType> interval);

    /**
     * \brief Add a date timestamp.
     */
    Key& Timestamp(TimestampType timestamp);

    /**
     * \brief Key device.
     */
    Key& Device(DeviceType device);

    /**
     * \brief Add position x.
     */
    Key& PositionX(double x_pos);

    /**
     * \brief Add position y.
     */
    Key& PositionY(double y_pos);

    /**
     * \brief Add action.
     */
    Key& Action(ButtonAction action);

    /**
     * \brief Add id.
     */
    Key& Button(int button);

    /**
     * \brief Add id.
     */
    Key& Name(std::string name);

    /**
     * \brief Add character.
     */
    Key& Character(std::string character);

    /**
     * \brief Add the device name.
     */
    Key& DeviceName(std::string device_name);

    /**
     * \brief Add the focus window name.
     */
    Key& FocusWindowName(std::string name);

    /**
     * \brief Add the focus window position x.
     */
    Key& FocusWindowPositionX(double x_pos);

    /**
     * \brief Add the focus window position y.
     */
    Key& FocusWindowPositionY(double y_pos);

    /**
     * \brief Add the focus window width.
     */
    Key& FocusWindowWidth(double width);

    /**
     * \brief Add the focus window height.
     */
    Key& FocusWindowHeight(double height);

    /**
     * \brief Add the screen number.
     */
    Key& Screen(int screen);

    /**
     * \brief Add modifier value.
     */
    Key& Modifier(ModifierValue modifier);

    /**
     * \brief Build key event.
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
