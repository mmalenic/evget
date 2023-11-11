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

#include "evgetcore/Event/Key.h"

#include <utility>

EvgetCore::Event::Key& EvgetCore::Event::Key::interval(Interval interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::interval(std::optional<Interval> interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::timestamp(Timestamp timestamp) {
    _timestamp = timestamp;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::device(EvgetCore::Event::Device device) {
    _device = device;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::positionX(double x) {
    _positionX = x;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::positionY(double y) {
    _positionY = y;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::action(ButtonAction action) {
    _action = action;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::button(int button) {
    _button = button;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::name(std::string name) {
    _name = std::move(name);
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::character(std::string character) {
    _character = std::move(character);
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::deviceName(std::string deviceName) {
    _deviceName = std::move(deviceName);
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::focusWindowName(std::string name) {
    _focusWindowName = std::move(name);
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::focusWindowPositionX(double x) {
    _focusWindowPositionX = x;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::focusWindowPositionY(double y) {
    _focusWindowPositionY = y;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::focusWindowWidth(double width) {
    _focusWindowWidth = width;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::focusWindowHeight(double height) {
    _focusWindowHeight = height;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::info(std::string info) {
    _info = std::move(info);
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::modifier(EvgetCore::Event::ModifierValue modifierValue) {
    _modifier = _modifier.modifierValue(modifierValue);
    return *this;
}

EvgetCore::Event::Data& EvgetCore::Event::Key::build(Data& data) {
    data.addNode(
        getName(),
        {{INTERVAL_FIELD, fromInterval(_interval)},
         {TIMESTAMP_FIELD, fromTimestamp(_timestamp)},
         {DEVICE_TYPE_FIELD, fromDevice(_device)},
         {POSITIONX_FIELD, fromDouble(_positionX)},
         {POSITIONY_FIELD, fromDouble(_positionY)},
         {ACTION_FIELD, fromButtonAction(_action)},
         {IDENTIFIER_FIELD, fromInt(_button)},
         {NAME_FIELD, fromString(_name)},
         {CHARACTER_FIELD, fromString(_character)},
         {DEVICE_NAME_FIELD, fromString(_deviceName)},
         {FOCUS_WINDOW_NAME_FIELD, fromString(_focusWindowName)},
         {FOCUS_WINDOW_POSITION_X_FIELD, fromDouble(_focusWindowPositionX)},
         {FOCUS_WINDOW_POSITION_Y_FIELD, fromDouble(_focusWindowPositionY)},
         {FOCUS_WINDOW_WIDTH_FIELD, fromDouble(_focusWindowWidth)},
         {FOCUS_WINDOW_HEIGHT_FIELD, fromDouble(_focusWindowHeight)},
         {INFO_FIELD, fromString(_info)}}
    );

    _modifier.build(data);

    data.addEdge(getName(), _modifier.getName(), Relation::ManyToMany);

    return data;
}