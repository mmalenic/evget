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

#include "evget/Event/MouseWheel.h"

Event::MouseWheel::MouseWheelBuilder::MouseWheelBuilder() :
_time{std::make_unique<Common::Time>()},
_wheelDown{std::make_unique<Cursor::WheelDown>()},
_wheelUp{std::make_unique<Cursor::WheelUp>()},
_isTouchPad{std::make_unique<Cursor::IsTouchPad>()} {
}

Event::MouseWheel::MouseWheelBuilder& Event::MouseWheel::MouseWheelBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

Event::MouseWheel::MouseWheelBuilder& Event::MouseWheel::MouseWheelBuilder::wheelDown(int amount) {
    _wheelDown = std::make_unique<Cursor::WheelDown>(amount);
    return *this;
}

Event::MouseWheel::MouseWheelBuilder& Event::MouseWheel::MouseWheelBuilder::wheelUp(int amount) {
    _wheelUp = std::make_unique<Cursor::WheelUp>(amount);
    return *this;
}

Event::MouseWheel::MouseWheelBuilder& Event::MouseWheel::MouseWheelBuilder::touchPad(bool isTouchPad) {
    _isTouchPad = std::make_unique<Cursor::IsTouchPad>(isTouchPad);
    return *this;
}

Event::MouseWheel Event::MouseWheel::MouseWheelBuilder::build() {
    return Event::MouseWheel(*this);
}

Event::MouseWheel::MouseWheel(
    Event::MouseWheel::MouseWheelBuilder& builder
) : AbstractData{"MouseWheel"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._wheelDown));
    fields.emplace_back(std::move(builder._wheelUp));
    fields.emplace_back(std::move(builder._isTouchPad));
}