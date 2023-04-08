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
     * Add scroll down.
     */
    MouseScroll& down(double amount);

    /**
     * Add scroll left.
     */
    MouseScroll& left(double amount);

    /**
     * Add scroll right.
     */
    MouseScroll& right(double amount);

    /**
     * Add scroll up.
     */
    MouseScroll& up(double amount);

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

    /**
     * Get mouse scroll name value.
     */
    static constexpr std::string getName();

    /**
     * Update the mouse wheel schema.
     */
    static constexpr void updateSchema(Schema& schema);

private:
    std::optional<Interval> _interval{};
    std::optional<Timestamp> _timestamp{};
    std::optional<Device> _device{};
    std::optional<double> _positionX{};
    std::optional<double> _positionY{};
    std::optional<double> _down{};
    std::optional<double> _left{};
    std::optional<double> _right{};
    std::optional<double> _up{};
    std::optional<std::string> _deviceName{};
    std::optional<std::string> _focusWindowName{};
    std::optional<double> _focusWindowPositionX{};
    std::optional<double> _focusWindowPositionY{};
    std::optional<double> _focusWindowWidth{};
    std::optional<double> _focusWindowHeight{};
    std::optional<std::string> _info{};

    Modifier _modifier{};
};

constexpr std::string MouseScroll::getName() {
    return "MouseScroll";
}

constexpr void EvgetCore::Event::MouseScroll::updateSchema(Schema& schema) {
    schema.addNode(
        getName(),
        {INTERVAL_FIELD,
         TIMESTAMP_FIELD,
         DEVICE_TYPE_FIELD,
         POSITIONX_FIELD,
         POSITIONY_FIELD,
         SCROLLDOWN_FIELD,
         SCROLLLEFT_FIELD,
         SCROLLRIGHT_FIELD,
         SCROLLUP_FIELD,
         DEVICE_NAME_FIELD,
         FOCUS_WINDOW_NAME_FIELD,
         FOCUS_WINDOW_POSITION_X_FIELD,
         FOCUS_WINDOW_POSITION_Y_FIELD,
         FOCUS_WINDOW_WIDTH_FIELD,
         FOCUS_WINDOW_HEIGHT_FIELD,
         INFO_FIELD}
    );

    schema.addEdge(getName(), Modifier::getName(), Relation::ManyToMany);
}
}  // namespace EvgetCore::Event

#endif  // EVGET_SRC_EVENT_MOUSEWHEEL_H
