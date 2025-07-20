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

#include <optional>
#include <string>
#include <utility>

#include "evgetcore/Event/ButtonAction.h"
#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Device.h"
#include "evgetcore/Event/Entry.h"
#include "evgetcore/Event/ModifierValue.h"
#include "evgetcore/Event/Schema.h"

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

EvgetCore::Event::Key& EvgetCore::Event::Key::positionX(double x_pos) {
    _positionX = x_pos;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::positionY(double y_pos) {
    _positionY = y_pos;
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

EvgetCore::Event::Key& EvgetCore::Event::Key::focusWindowPositionX(double x_pos) {
    _focusWindowPositionX = x_pos;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::focusWindowPositionY(double y_pos) {
    _focusWindowPositionY = y_pos;
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

EvgetCore::Event::Key& EvgetCore::Event::Key::screen(int screen) {
    _screen = screen;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::modifier(EvgetCore::Event::ModifierValue modifierValue) {
    _modifiers.push_back(toUnderlying(modifierValue));
    return *this;
}

EvgetCore::Event::Data& EvgetCore::Event::Key::build(Data& data) const {
    const Entry entry{
        EntryType::Key,
        {
            fromInterval(_interval),
            fromTimestamp(_timestamp),
            fromDouble(_positionX),
            fromDouble(_positionY),
            fromString(_deviceName),
            fromString(_focusWindowName),
            fromDouble(_focusWindowPositionX),
            fromDouble(_focusWindowPositionY),
            fromDouble(_focusWindowWidth),
            fromDouble(_focusWindowHeight),
            fromInt(_screen),
            toUnderlyingOptional(_device),
            fromInt(_button),
            fromString(_name),
            toUnderlyingOptional(_action),
            fromString(_character),
        },
        _modifiers
    };

    data.addEntry(entry);

    return data;
}
