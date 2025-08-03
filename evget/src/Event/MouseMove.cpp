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

#include "evget/Event/MouseMove.h"

#include <optional>
#include <string>
#include <utility>

#include "evget/Event/Data.h"
#include "evget/Event/Device.h"
#include "evget/Event/Entry.h"
#include "evget/Event/ModifierValue.h"
#include "evget/Event/Schema.h"

evget::Event::MouseMove& evget::Event::MouseMove::interval(Interval interval) {
    _interval = interval;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::interval(std::optional<Interval> interval) {
    _interval = interval;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::timestamp(Timestamp timestamp) {
    _timestamp = timestamp;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::device(evget::Event::Device device) {
    _device = device;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::positionX(double x_pos) {
    _positionX = x_pos;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::positionY(double y_pos) {
    _positionY = y_pos;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::deviceName(std::string deviceName) {
    _deviceName = std::move(deviceName);
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::focusWindowName(std::string name) {
    _focusWindowName = std::move(name);
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::focusWindowPositionX(double x_pos) {
    _focusWindowPositionX = x_pos;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::focusWindowPositionY(double y_pos) {
    _focusWindowPositionY = y_pos;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::focusWindowWidth(double width) {
    _focusWindowWidth = width;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::focusWindowHeight(double height) {
    _focusWindowHeight = height;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::screen(int screen) {
    _screen = screen;
    return *this;
}

evget::Event::MouseMove& evget::Event::MouseMove::modifier(evget::Event::ModifierValue modifierValue) {
    _modifiers.push_back(toUnderlying(modifierValue));
    return *this;
}

evget::Event::Data& evget::Event::MouseMove::build(Data& data) const {
    const Entry entry{
        EntryType::MouseMove,
        {fromInterval(_interval),
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
         toUnderlyingOptional(_device)},
        _modifiers
    };

    data.addEntry(entry);

    return data;
}
