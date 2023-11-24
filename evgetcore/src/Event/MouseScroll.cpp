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

#include "evgetcore/Event/MouseScroll.h"

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::interval(Interval interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::interval(std::optional<Interval> interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::timestamp(Timestamp timestamp) {
    _timestamp = timestamp;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::device(EvgetCore::Event::Device device) {
    _device = device;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::positionX(double x) {
    _positionX = x;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::positionY(double y) {
    _positionY = y;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::vertical(double amount) {
    _vertical = amount;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::horizontal(double amount) {
    _horizontal = amount;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::deviceName(std::string deviceName) {
    _deviceName = std::move(deviceName);
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::focusWindowName(std::string name) {
    _focusWindowName = std::move(name);
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::focusWindowPositionX(double x) {
    _focusWindowPositionX = x;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::focusWindowPositionY(double y) {
    _focusWindowPositionY = y;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::focusWindowWidth(double width) {
    _focusWindowWidth = width;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::focusWindowHeight(double height) {
    _focusWindowHeight = height;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::info(std::string info) {
    _info = std::move(info);
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::modifier(EvgetCore::Event::ModifierValue modifierValue) {
    _modifiers.push_back(modifierValue);
    return *this;
}

EvgetCore::Event::Data& EvgetCore::Event::MouseScroll::build(Data& data) {
    Entry entry{
        .type = EntryType::MouseScroll,
        .data = {
            fromInterval(_interval),
            fromTimestamp(_timestamp),
            fromDevice(_device),
            fromDouble(_positionX),
            fromDouble(_positionY),
            fromDouble(_vertical),
            fromDouble(_horizontal),
            fromString(_deviceName),
            fromString(_focusWindowName),
            fromDouble(_focusWindowPositionX),
            fromDouble(_focusWindowPositionY),
            fromDouble(_focusWindowWidth),
            fromDouble(_focusWindowHeight),
            fromString(_info)
        },
        .modifiers = _modifiers
    };

    data.addEntry(entry);

    return data;
}
