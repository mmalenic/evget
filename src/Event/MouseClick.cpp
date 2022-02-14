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
#include "evget/Event/Pressable/ButtonType.h"

#include <utility>

Event::MouseClick::MouseClickBuilder::MouseClickBuilder() :
    _time{std::make_unique<Common::Time>()},
    _type{std::make_unique<Common::DeviceType>()},
    _positionX{std::make_unique<Pointer::PositionX>()},
    _positionY{std::make_unique<Pointer::PositionY>()},
    _buttonType{std::make_unique<Pressable::ButtonType>()},
    _button{std::make_unique<Pressable::Button>()}
{
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::positionX(double x) {
    _positionX = std::make_unique<Pointer::PositionX>(x);
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::positionY(double y) {
    _positionY = std::make_unique<Pointer::PositionY>(y);
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::action(Pressable::Action action) {
    _buttonType = Event::Pressable::ButtonType::createButtonType(action);
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::button(std::string button) {
    _button = std::make_unique<Pressable::Button>(std::move(button));
    return *this;
}

Event::MouseClick::MouseClickBuilder& Event::MouseClick::MouseClickBuilder::device(Event::Common::Device device) {
    _type = Event::Common::DeviceType::createType(device);
    return *this;
}

std::unique_ptr<Event::MouseClick> Event::MouseClick::MouseClickBuilder::build() {
    return std::make_unique<Event::MouseClick>(*this);
}

Event::MouseClick::MouseClick(
    Event::MouseClick::MouseClickBuilder& builder
) : AbstractData{"MouseClick"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._type));
    fields.emplace_back(std::move(builder._positionX));
    fields.emplace_back(std::move(builder._positionY));
    fields.emplace_back(std::move(builder._buttonType));
    fields.emplace_back(std::move(builder._button));
}