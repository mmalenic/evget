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
 * Represents a key event.
 */
class Key {
public:
    /**
     * Add interval in microseconds.
     */
    Key& Interval(IntervalType interval);

    /**
     * Add interval in microseconds.
     */
    Key& Interval(std::optional<IntervalType> interval);

    /**
     * Add date timestamp.
     */
    Key& Timestamp(TimestampType timestamp);

    /**
     * Key device.
     */
    Key& Device(DeviceType device);

    /**
     * Add position x.
     */
    Key& PositionX(double x_pos);

    /**
     * Add position y.
     */
    Key& PositionY(double y_pos);

    /**
     * Add action.
     */
    Key& Action(ButtonAction action);

    /**
     * Add id.
     */
    Key& Button(int button);

    /**
     * Add id.
     */
    Key& Name(std::string name);

    /**
     * Add character.
     */
    Key& Character(std::string character);

    /**
     * Add the device name.
     */
    Key& DeviceName(std::string device_name);

    /**
     * Add the focus window name.
     */
    Key& FocusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    Key& FocusWindowPositionX(double x_pos);

    /**
     * Add the focus window position y.
     */
    Key& FocusWindowPositionY(double y_pos);

    /**
     * Add the focus window width.
     */
    Key& FocusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    Key& FocusWindowHeight(double height);

    /**
     * Add the screen number.
     */
    Key& Screen(int screen);

    /**
     * Add modifier value.
     */
    Key& Modifier(ModifierValue modifier);

    /**
     * Build key event.
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
