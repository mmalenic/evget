// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef EVGET_SRC_EVENT_MOUSECLICK_H
#define EVGET_SRC_EVENT_MOUSECLICK_H

#include <evget/Event/ButtonAction.h>
#include <evget/Event/DeviceType.h>
#include <evget/Event/ModifierValue.h>
#include <optional>
#include <string>
#include <vector>

#include "evget/Event/Data.h"
#include "evget/Event/Schema.h"

namespace evget::event {
/**
 * Represents a mouse click event.
 */
class MouseClick {
public:
    /**
     * Add interval in microseconds.
     */
    MouseClick& Interval(IntervalType interval);

    /**
     * Add interval in microseconds.
     */
    MouseClick& Interval(std::optional<IntervalType> interval);

    /**
     * Add date timestamp.
     */
    MouseClick& Timestamp(TimestampType timestamp);

    /**
     * Mouse device.
     */
    MouseClick& Device(DeviceType device);

    /**
     * Add position x.
     */
    MouseClick& PositionX(double x_pos);

    /**
     * Add position y.
     */
    MouseClick& PositionY(double y_pos);

    /**
     * Add action.
     */
    MouseClick& Action(ButtonAction action);

    /**
     * Add id.
     */
    MouseClick& Button(int button);

    /**
     * Add name.
     */
    MouseClick& Name(std::string name);

    /**
     * Add the device name.
     */
    MouseClick& DeviceName(std::string device_name);

    /**
     * Add the focus window name.
     */
    MouseClick& FocusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    MouseClick& FocusWindowPositionX(double x_pos);

    /**
     * Add the focus window position y.
     */
    MouseClick& FocusWindowPositionY(double y_pos);

    /**
     * Add the focus window width.
     */
    MouseClick& FocusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    MouseClick& FocusWindowHeight(double height);

    /**
     * Add the screen number.
     */
    MouseClick& Screen(int screen);

    /**
     * Add modifier value.
     */
    MouseClick& Modifier(ModifierValue modifier);

    /**
     * Build mouse click event.
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
}  // namespace evget::event

#endif  // EVGET_SRC_EVENT_MOUSECLICK_H
