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

#include "evget/Event/MouseScroll.h"

Event::MouseScroll::MouseScrollBuilder::MouseScrollBuilder() :
_time{std::make_unique<Common::Time>()},
_device{std::make_unique<Common::DeviceType>()},
_up{std::make_unique<Pointer::ScrollUp>()},
_down{std::make_unique<Pointer::ScrollDown>()},
_left{std::make_unique<Pointer::ScrollLeft>()},
_right{std::make_unique<Pointer::ScrollRight>()} {
}

Event::MouseScroll::MouseScrollBuilder& Event::MouseScroll::MouseScrollBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

Event::MouseScroll::MouseScrollBuilder& Event::MouseScroll::MouseScrollBuilder::device(Event::Common::Device device) {
    _device = Event::Common::DeviceType::createType(device);
    return *this;
}

Event::MouseScroll::MouseScrollBuilder& Event::MouseScroll::MouseScrollBuilder::up(double amount) {
    _up = std::make_unique<Event::Pointer::ScrollUp>(amount);
    return *this;
}

Event::MouseScroll::MouseScrollBuilder& Event::MouseScroll::MouseScrollBuilder::down(double amount) {
    _down = std::make_unique<Event::Pointer::ScrollDown>(amount);
    return *this;
}

Event::MouseScroll::MouseScrollBuilder& Event::MouseScroll::MouseScrollBuilder::left(double amount) {
    _left = std::make_unique<Event::Pointer::ScrollLeft>(amount);
    return *this;
}

Event::MouseScroll::MouseScrollBuilder& Event::MouseScroll::MouseScrollBuilder::right(double amount) {
    _right = std::make_unique<Event::Pointer::ScrollRight>(amount);
    return *this;
}

std::unique_ptr<Event::MouseScroll> Event::MouseScroll::MouseScrollBuilder::build() {
    return std::make_unique<Event::MouseScroll>(*this);
}

Event::MouseScroll::MouseScroll(
    Event::MouseScroll::MouseScrollBuilder& builder
) : AbstractData{"MouseScroll"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._device));
    fields.emplace_back(std::move(builder._up));
    fields.emplace_back(std::move(builder._down));
    fields.emplace_back(std::move(builder._left));
    fields.emplace_back(std::move(builder._right));
}