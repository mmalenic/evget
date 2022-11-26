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

#include "evgetcore/Event/MouseMove.h"

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::interval(SchemaField::Interval interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::interval(std::optional<SchemaField::Interval> interval) {
    _interval = interval;
    return *this;
}

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::timestamp(SchemaField::Timestamp timestamp) {
    _timestamp = timestamp;
    return *this;
}

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::device(EvgetCore::Event::Device device) {
    _device = device;
    return *this;
}

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::positionX(double x) {
    _positionX = x;
    return *this;
}

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::positionY(double y) {
    _positionY = y;
    return *this;
}

EvgetCore::Event::MouseMove &EvgetCore::Event::MouseMove::focusWindowName(std::string name) {
    _focusWindowName = std::move(name);
    return *this;
}

EvgetCore::Event::MouseMove &EvgetCore::Event::MouseMove::focusWindowPositionX(double x) {
    _focusWindowPositionX = x;
    return *this;
}

EvgetCore::Event::MouseMove &EvgetCore::Event::MouseMove::focusWindowPositionY(double y) {
    _focusWindowPositionY = y;
    return *this;
}

EvgetCore::Event::MouseMove &EvgetCore::Event::MouseMove::focusWindowWidth(double width) {
    _focusWindowWidth = width;
    return *this;
}

EvgetCore::Event::MouseMove &EvgetCore::Event::MouseMove::focusWindowHeight(double height) {
    _focusWindowHeight = height;
    return *this;
}

EvgetCore::Event::Data EvgetCore::Event::MouseMove::build() {
    auto data = Data{"MouseMove"};

    data.addField(SchemaField::fromInterval(_interval));
    data.addField(SchemaField::fromTimestamp(_timestamp));
    data.addField(SchemaField::fromDevice(_device));
    data.addField(SchemaField::fromDouble(_positionX));
    data.addField(SchemaField::fromDouble(_positionY));
    data.addField(SchemaField::fromString(_focusWindowName));
    data.addField(SchemaField::fromDouble(_focusWindowPositionX));
    data.addField(SchemaField::fromDouble(_focusWindowPositionY));
    data.addField(SchemaField::fromDouble(_focusWindowWidth));
    data.addField(SchemaField::fromDouble(_focusWindowHeight));

    return data;
}
