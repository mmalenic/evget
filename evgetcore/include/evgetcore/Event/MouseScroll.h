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

#include <chrono>

#include "Data.h"
#include "Modifier.h"
#include "Schema.h"

namespace EvgetCore::Event {
/**
 * Represents a mouse wheel event.
 */
class MouseScroll {
public:
    /**
     * Add interval in microseconds.
     */
    MouseScroll& interval(Interval interval);

    /**
     * Add interval in microseconds.
     */
    MouseScroll& interval(std::optional<Interval> interval);

    /**
     * Add date timestamp
     */
    MouseScroll& timestamp(Timestamp timestamp);

    /**
     * Mouse device.
     */
    MouseScroll& device(Device device);

    /**
     * Add position x.
     */
    MouseScroll& positionX(double x);

    /**
     * Add position y.
     */
    MouseScroll& positionY(double y);

    /**
     * Add vertical scroll.
     */
    MouseScroll& vertical(double amount);

    /**
     * Add horizontal scroll.
     */
    MouseScroll& horizontal(double amount);

    /**
     * Add the device name.
     */
    MouseScroll& deviceName(std::string deviceName);

    /**
     * Add the focus window name.
     */
    MouseScroll& focusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    MouseScroll& focusWindowPositionX(double x);

    /**
     * Add the focus window position y.
     */
    MouseScroll& focusWindowPositionY(double y);

    /**
     * Add the focus window width.
     */
    MouseScroll& focusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    MouseScroll& focusWindowHeight(double height);

    /**
     * Add optional user configured info.
     */
    MouseScroll& info(std::string info);

    /**
     * Add modifier value.
     */
    MouseScroll& modifier(ModifierValue modifier);

    /**
     * Build mouse wheel event.
     */
    Data& build(Data& data);

private:
    std::optional<Interval> _interval{};
    std::optional<Timestamp> _timestamp{};
    std::optional<Device> _device{};
    std::optional<double> _positionX{};
    std::optional<double> _positionY{};
    std::optional<double> _vertical{};
    std::optional<double> _horizontal{};
    std::optional<std::string> _deviceName{};
    std::optional<std::string> _focusWindowName{};
    std::optional<double> _focusWindowPositionX{};
    std::optional<double> _focusWindowPositionY{};
    std::optional<double> _focusWindowWidth{};
    std::optional<double> _focusWindowHeight{};
    std::optional<std::string> _info{};

    std::vector<ModifierValue> _modifiers{};
};
}  // namespace EvgetCore::Event

#endif  // EVGET_SRC_EVENT_MOUSEWHEEL_H
