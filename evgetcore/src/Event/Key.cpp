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

EvgetCore::Event::Key& EvgetCore::Event::Key::time(std::chrono::nanoseconds nanoseconds) {
    _time = nanoseconds;
    return *this;
}

EvgetCore::Event::Key& EvgetCore::Event::Key::dateTime(Schema::DateTime dateTime) {
    _dateTime = dateTime;
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

EvgetCore::Event::Data EvgetCore::Event::Key::build() {
    auto data = Data{"Key"};

    data.addField(Schema::fromNanoseconds(_time));
    data.addField(Schema::fromDateTime(_dateTime));
    data.addField(Schema::fromDevice(_device));
    data.addField(Schema::fromDouble(_positionX));
    data.addField(Schema::fromDouble(_positionY));
    data.addField(Schema::fromButtonAction(_action));
    data.addField(Schema::fromInt(_button));
    data.addField(Schema::fromString(_name));
    data.addField(Schema::fromString(_character));

    return data;
}

constexpr EvgetCore::Event::Schema EvgetCore::Event::Key::generateSchema() {
    return Schema{""};
}
