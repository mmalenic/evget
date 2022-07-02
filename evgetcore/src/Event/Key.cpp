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
#include "evgetcore/Event/Button/Action.h"

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::device(EvgetCore::Event::Common::Device device) {
    _device = EvgetCore::Event::Common::DeviceType::createType(device);
    return *this;
}

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::positionX(double x) {
    _positionX = std::make_unique<Pointer::PositionX>(x);
    return *this;
}

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::positionY(double y) {
    _positionY = std::make_unique<Pointer::PositionY>(y);
    return *this;
}

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::action(Button::ButtonAction action) {
    _buttonType = EvgetCore::Event::Button::Action::createAction(action);
    return *this;
}

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::button(int button) {
    _button = std::make_unique<Button::Identifier>(button);
    return *this;
}

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::name(std::string name) {
    _name = std::make_unique<Button::Name>(std::move(name));
    return *this;
}

EvgetCore::Event::Key::KeyBuilder& EvgetCore::Event::Key::KeyBuilder::character(const std::string& character) {
    _character = std::make_unique<Button::Character>(character);
    return *this;
}

std::unique_ptr<EvgetCore::Event::Key> EvgetCore::Event::Key::KeyBuilder::build() {
    return std::make_unique<EvgetCore::Event::Key>(*this);
}

EvgetCore::Event::Key::Key(
    EvgetCore::Event::Key::KeyBuilder& builder
) : AbstractData{"Key"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._device));
    fields.emplace_back(std::move(builder._positionX));
    fields.emplace_back(std::move(builder._positionY));
    fields.emplace_back(std::move(builder._buttonType));
    fields.emplace_back(std::move(builder._button));
    fields.emplace_back(std::move(builder._name));
    fields.emplace_back(std::move(builder._character));
}