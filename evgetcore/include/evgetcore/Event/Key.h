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

#ifndef EVGET_SRC_KEY_H
#define EVGET_SRC_KEY_H

#include <c++/11/array>
#include <chrono>

#include "ButtonAction.h"
#include "Modifier.h"
#include "Schema.h"

namespace EvgetCore::Event {
/**
 * Represents a key event.
 */
class Key {
public:
    /**
     * Add interval in microseconds.
     */
    Key& interval(Interval interval);

    /**
     * Add interval in microseconds.
     */
    Key& interval(std::optional<Interval> interval);

    /**
     * Add date timestamp.
     */
    Key& timestamp(Timestamp dateTime);

    /**
     * Key device.
     */
    Key& device(Device device);

    /**
     * Add position x.
     */
    Key& positionX(double x);

    /**
     * Add position y.
     */
    Key& positionY(double y);

    /**
     * Add action.
     */
    Key& action(ButtonAction action);

    /**
     * Add id.
     */
    Key& button(int button);

    /**
     * Add id.
     */
    Key& name(std::string name);

    /**
     * Add character.
     */
    Key& character(std::string character);

    /**
     * Add the device name.
     */
    Key& deviceName(std::string deviceName);

    /**
     * Add the focus window name.
     */
    Key& focusWindowName(std::string name);

    /**
     * Add the focus window position x.
     */
    Key& focusWindowPositionX(double x);

    /**
     * Add the focus window position y.
     */
    Key& focusWindowPositionY(double y);

    /**
     * Add the focus window width.
     */
    Key& focusWindowWidth(double width);

    /**
     * Add the focus window height.
     */
    Key& focusWindowHeight(double height);

    /**
     * Add optional user configured info.
     */
    Key& info(std::string info);

    /**
     * Add modifier value.
     */
    Key& modifier(ModifierValue modifier);

    /**
     * Build key event.
     */
    Data& build(Data& data);

    /**
     * Get key name value.
     */
    static constexpr std::string getName();

    /**
     * Update the key schema.
     */
    static constexpr void updateSchema(Schema& schema);

private:
    std::optional<Interval> _interval{};
    std::optional<Timestamp> _timestamp{};
    std::optional<Device> _device{};
    std::optional<double> _positionX{};
    std::optional<double> _positionY{};
    std::optional<ButtonAction> _action{};
    std::optional<int> _button{};
    std::optional<std::string> _name{};
    std::optional<std::string> _character{};
    std::optional<std::string> _deviceName{};
    std::optional<std::string> _focusWindowName{};
    std::optional<double> _focusWindowPositionX{};
    std::optional<double> _focusWindowPositionY{};
    std::optional<double> _focusWindowWidth{};
    std::optional<double> _focusWindowHeight{};
    std::optional<std::string> _info{};

    Modifier _modifier{};
};

constexpr std::string Key::getName() {
    return "Key";
}

constexpr void EvgetCore::Event::Key::updateSchema(Schema& schema) {
    schema.addNode(
        getName(),
        {INTERVAL_FIELD,
         TIMESTAMP_FIELD,
         DEVICE_TYPE_FIELD,
         POSITIONX_FIELD,
         POSITIONY_FIELD,
         ACTION_FIELD,
         IDENTIFIER_FIELD,
         NAME_FIELD,
         CHARACTER_FIELD,
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

#endif  // EVGET_SRC_KEY_H
