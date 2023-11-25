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

#include "evgetcore/Event/MouseClick.h"

#include <utility>

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::interval(Interval interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::interval(std::optional<Interval> interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::timestamp(Timestamp timestamp) {
    _timestamp = timestamp;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::device(EvgetCore::Event::Device device) {
    _device = device;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::positionX(double x) {
    _positionX = x;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::positionY(double y) {
    _positionY = y;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::action(ButtonAction action) {
    _action = action;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::button(int button) {
    _button = button;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::name(std::string name) {
    _name = std::move(name);
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::deviceName(std::string deviceName) {
    _deviceName = std::move(deviceName);
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::focusWindowName(std::string name) {
    _focusWindowName = std::move(name);
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::focusWindowPositionX(double x) {
    _focusWindowPositionX = x;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::focusWindowPositionY(double y) {
    _focusWindowPositionY = y;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::focusWindowWidth(double width) {
    _focusWindowWidth = width;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::focusWindowHeight(double height) {
    _focusWindowHeight = height;
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::info(std::string info) {
    _info = std::move(info);
    return *this;
}

EvgetCore::Event::MouseClick& EvgetCore::Event::MouseClick::modifier(EvgetCore::Event::ModifierValue modifierValue) {
    _modifiers.push_back(toUnderlying(modifierValue));
    return *this;
}

EvgetCore::Event::Data& EvgetCore::Event::MouseClick::build(Data& data) {
    Entry entry{
        EntryType::MouseClick,
        {
            fromInterval(_interval),
            fromTimestamp(_timestamp),
            toUnderlyingOptional(_device),
            fromDouble(_positionX),
            fromDouble(_positionY),
            fromString(_deviceName),
            fromString(_focusWindowName),
            fromDouble(_focusWindowPositionX),
            fromDouble(_focusWindowPositionY),
            fromDouble(_focusWindowWidth),
            fromDouble(_focusWindowHeight),
            fromString(_info),
            toUnderlyingOptional(_action),
            fromInt(_button),
            fromString(_name),
        },
        _modifiers
    };

    data.addEntry(entry);

    return data;
}
