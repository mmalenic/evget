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

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::time(std::chrono::nanoseconds nanoseconds) {
    _time = nanoseconds;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::dateTime(SchemaField::Timestamp dateTime) {
    _dateTime = dateTime;
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

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::down(double amount) {
    _down = amount;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::left(double amount) {
    _left = amount;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::right(double amount) {
    _right = amount;
    return *this;
}

EvgetCore::Event::MouseScroll& EvgetCore::Event::MouseScroll::up(double amount) {
    _up = amount;
    return *this;
}

EvgetCore::Event::Data EvgetCore::Event::MouseScroll::build() {
    auto data = Data{"MouseScroll"};

    data.addField(SchemaField::fromNanoseconds(_time));
    data.addField(SchemaField::fromTimestamp(_dateTime));
    data.addField(SchemaField::fromDevice(_device));
    data.addField(SchemaField::fromDouble(_positionX));
    data.addField(SchemaField::fromDouble(_positionY));
    data.addField(SchemaField::fromDouble(_down));
    data.addField(SchemaField::fromDouble(_left));
    data.addField(SchemaField::fromDouble(_right));
    data.addField(SchemaField::fromDouble(_up));

    return data;
}
