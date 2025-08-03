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

#include "evget/Event/MouseClick.h"

#include <optional>
#include <string>
#include <utility>

#include "evget/Event/ButtonAction.h"
#include "evget/Event/Data.h"
#include "evget/Event/Device.h"
#include "evget/Event/Entry.h"
#include "evget/Event/ModifierValue.h"
#include "evget/Event/Schema.h"

evget::Event::MouseClick& evget::Event::MouseClick::interval(Interval interval) {
    _interval = interval;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::interval(std::optional<Interval> interval) {
    _interval = interval;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::timestamp(Timestamp timestamp) {
    _timestamp = timestamp;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::device(evget::Event::Device device) {
    _device = device;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::positionX(double x_pos) {
    _positionX = x_pos;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::positionY(double y_pos) {
    _positionY = y_pos;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::action(ButtonAction action) {
    _action = action;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::button(int button) {
    _button = button;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::name(std::string name) {
    _name = std::move(name);
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::deviceName(std::string deviceName) {
    _deviceName = std::move(deviceName);
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::focusWindowName(std::string name) {
    _focusWindowName = std::move(name);
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::focusWindowPositionX(double x_pos) {
    _focusWindowPositionX = x_pos;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::focusWindowPositionY(double y_pos) {
    _focusWindowPositionY = y_pos;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::focusWindowWidth(double width) {
    _focusWindowWidth = width;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::focusWindowHeight(double height) {
    _focusWindowHeight = height;
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::modifier(evget::Event::ModifierValue modifierValue) {
    _modifiers.push_back(toUnderlying(modifierValue));
    return *this;
}

evget::Event::MouseClick& evget::Event::MouseClick::screen(int screen) {
    _screen = screen;
    return *this;
}

evget::Event::Data& evget::Event::MouseClick::build(Data& data) const {
    const Entry entry{
        EntryType::MouseClick,
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
        },
        _modifiers
    };

    data.addEntry(entry);

    return data;
}
