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

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::time(std::chrono::nanoseconds nanoseconds) {
    _time = nanoseconds;
    return *this;
}

EvgetCore::Event::MouseMove& EvgetCore::Event::MouseMove::dateTime(SchemaField::Timestamp dateTime) {
    _dateTime = dateTime;
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

EvgetCore::Event::Data EvgetCore::Event::MouseMove::build() {
    auto data = Data{"MouseMove"};

    data.addField(SchemaField::fromNanoseconds(_time));
    data.addField(SchemaField::fromTimestamp(_dateTime));
    data.addField(SchemaField::fromDevice(_device));
    data.addField(SchemaField::fromDouble(_positionX));
    data.addField(SchemaField::fromDouble(_positionY));

    return data;
}