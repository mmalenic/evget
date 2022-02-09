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

#include <utility>

Event::MouseClick::MouseClickBuilder::MouseClickBuilder() :
    _time{std::make_unique<Common::Time>()},
    _type{std::make_unique<Common::Type>()},
    _positionX{std::make_unique<Cursor::PositionX>()},
    _positionY{std::make_unique<Cursor::PositionY>()},
    _press{std::make_unique<Button::Press>()},
    _release{std::make_unique<Button::Release>()}
{
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::positionX(int x) {
    _positionX = std::make_unique<Cursor::PositionX>(x);
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::positionY(int y) {
    _positionY = std::make_unique<Cursor::PositionY>(y);
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::press(std::string button) {
    _press = std::make_unique<Button::Press>(std::move(button));
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::release(std::string button) {
    _release = std::make_unique<Button::Release>(std::move(button));
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::type(Event::Common::Type::Device device) {
    _type = Event::Common::Type::createType(device);
    return *this;
}

Event::MouseClick Event::MouseClick::MouseClickBuilder::build() {
    return Event::MouseClick(*this);
}

Event::MouseClick::MouseClick(
    Event::MouseClick::MouseClickBuilder& builder
) : AbstractData{"MouseClick"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._type));
    fields.emplace_back(std::move(builder._positionX));
    fields.emplace_back(std::move(builder._positionY));
    fields.emplace_back(std::move(builder._press));
    fields.emplace_back(std::move(builder._release));
}