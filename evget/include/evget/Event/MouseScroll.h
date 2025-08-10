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

#ifndef EVGET_SRC_EVENT_MOUSEWHEEL_H
#define EVGET_SRC_EVENT_MOUSEWHEEL_H

#include <evget/Event/DeviceType.h>
#include <evget/Event/ModifierValue.h>
#include <optional>
#include <string>
#include <vector>

#include "evget/Event/Data.h"
#include "evget/Event/Schema.h"

namespace evget::event {
/**
 * Represents a mouse wheel event.
 */
class MouseScroll {
public:
    /**
     * Add interval in microseconds.
     */
    MouseScroll& Interval(IntervalType interval);

    /**
     * Add interval in microseconds.
     */
    MouseScroll& Interval(std::optional<IntervalType> interval);

    /**
     * Add date timestamp
     */
    MouseScroll& Timestamp(TimestampType timestamp);

    /**
     * Mouse device.
     */
    MouseScroll& Device(DeviceType device);

    /**
     * Add position x.
     */
    MouseScroll& PositionX(double x_pos);

    /**
     * Add position y.
     */
    MouseScroll& PositionY(double y_pos);

    /**
     * Add vertical scroll.
     */
    MouseScroll& Vertical(double amount);

    /**
     * Add horizontal scroll.
     */
    MouseScroll& Horizontal(double amount);

    /**
     * Add the device name.
     */
    MouseScroll& DeviceName(std::string device_name);

    /**
     * Add the focus window name.
     */
    MouseScroll& FocusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    MouseScroll& FocusWindowPositionX(double x_pos);

    /**
     * Add the focus window position y.
     */
    MouseScroll& FocusWindowPositionY(double y_pos);

    /**
     * Add the focus window width.
     */
    MouseScroll& FocusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    MouseScroll& FocusWindowHeight(double height);

    /**
     * Add the screen number.
     */
    MouseScroll& Screen(int screen);

    /**
     * Add modifier value.
     */
    MouseScroll& Modifier(ModifierValue modifier);

    /**
     * Build mouse wheel event.
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
}  // namespace evget::event

#endif  // EVGET_SRC_EVENT_MOUSEWHEEL_H
