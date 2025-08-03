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

#ifndef EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H
#define EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "evget/Event/Data.h"
#include "evget/Event/Schema.h"

namespace evget::Event {
/**
 * Represents a mouse move event.
 */
class MouseMove {
public:
    /**
     * Add interval in microseconds.
     */
    MouseMove& interval(Interval interval);

    /**
     * Add interval in microseconds.
     */
    MouseMove& interval(std::optional<Interval> interval);

    /**
     * Add date timestamp.
     */
    MouseMove& timestamp(Timestamp timestamp);

    /**
     * Mouse device.
     */
    MouseMove& device(Device device);

    /**
     * Add position x.
     */
    MouseMove& positionX(double x_pos);

    /**
     * Add position y.
     */
    MouseMove& positionY(double y_pos);

    /**
     * Add the device name.
     */
    MouseMove& deviceName(std::string deviceName);

    /**
     * Add the focus window name.
     */
    MouseMove& focusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    MouseMove& focusWindowPositionX(double x_pos);

    /**
     * Add the focus window position y.
     */
    MouseMove& focusWindowPositionY(double y_pos);

    /**
     * Add the focus window width.
     */
    MouseMove& focusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    MouseMove& focusWindowHeight(double height);

    /**
     * Add the screen number.
     */
    MouseMove& screen(int screen);

    /**
     * Add modifier value.
     */
    MouseMove& modifier(ModifierValue modifier);

    /**
     * Build mouse move event.
     */
    Data& build(Data& data) const;

private:
    std::optional<Interval> _interval;
    std::optional<Timestamp> _timestamp;
    std::optional<Device> _device;
    std::optional<double> _positionX;
    std::optional<double> _positionY;
    std::optional<std::string> _deviceName;
    std::optional<std::string> _focusWindowName;
    std::optional<double> _focusWindowPositionX;
    std::optional<double> _focusWindowPositionY;
    std::optional<double> _focusWindowWidth;
    std::optional<double> _focusWindowHeight;
    std::optional<int> _screen;

    std::vector<std::string> _modifiers;
};
}  // namespace evget::Event

#endif  // EVGET_INCLUDE_EVENT_DATA_MOUSEMOVE_H
