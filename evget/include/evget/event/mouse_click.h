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
     */
    MouseClick& Interval(IntervalType interval);

    /**
     * \brief Add an interval in microseconds.
     */
    MouseClick& Interval(std::optional<IntervalType> interval);

    /**
     * \brief Add a date timestamp.
     */
    MouseClick& Timestamp(TimestampType timestamp);

    /**
     * \brief Mouse device.
     */
    MouseClick& Device(DeviceType device);

    /**
     * \brief Add position x.
     */
    MouseClick& PositionX(double x_pos);

    /**
     * \brief Add position y.
     */
    MouseClick& PositionY(double y_pos);

    /**
     * \brief Add action.
     */
    MouseClick& Action(ButtonAction action);

    /**
     * \brief Add id.
     */
    MouseClick& Button(int button);

    /**
     * \brief Add name.
     */
    MouseClick& Name(std::string name);

    /**
     * \brief Add the device name.
     */
    MouseClick& DeviceName(std::string device_name);

    /**
     * \brief Add the focus window name.
     */
    MouseClick& FocusWindowName(std::string name);

    /**
     * \brief Add the focus window position x.
     */
    MouseClick& FocusWindowPositionX(double x_pos);

    /**
     * \brief Add the focus window position y.
     */
    MouseClick& FocusWindowPositionY(double y_pos);

    /**
     * \brief Add the focus window width.
     */
    MouseClick& FocusWindowWidth(double width);

    /**
     * \brief Add the focus window height.
     */
    MouseClick& FocusWindowHeight(double height);

    /**
     * \brief Add the screen number.
     */
    MouseClick& Screen(int screen);

    /**
     * \brief Add modifier value.
     */
    MouseClick& Modifier(ModifierValue modifier);

    /**
     * \brief Build mouse click event.
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
