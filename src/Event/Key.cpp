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

#include "evget/Event/Key.h"

Event::Key::KeyBuilder::KeyBuilder() :
_time{std::make_unique<Common::Time>()},
_press{std::make_unique<Button::Press>()},
_release{std::make_unique<Button::Release>()},
_repeat{std::make_unique<Button::Repeat>()},
_character{std::make_unique<Button::Character>()} {
}

Event::Key::KeyBuilder& Event::Key::KeyBuilder::time(std::chrono::nanoseconds nanoseconds) {
    _time = std::make_unique<Common::Time>(nanoseconds);
    return *this;
}

Event::Key::KeyBuilder& Event::Key::KeyBuilder::press(std::string button) {
    _press = std::make_unique<Button::Press>(std::move(button));
    return *this;
}

Event::Key::KeyBuilder& Event::Key::KeyBuilder::release(std::string button) {
    _release = std::make_unique<Button::Release>(std::move(button));
    return *this;
}

Event::Key::KeyBuilder& Event::Key::KeyBuilder::repeat(std::string button) {
    _repeat = std::make_unique<Button::Repeat>(std::move(button));
    return *this;
}

Event::Key::KeyBuilder& Event::Key::KeyBuilder::character(char character) {
    _character = std::make_unique<Button::Character>(character);
    return *this;
}

std::unique_ptr<Event::Key> Event::Key::KeyBuilder::build() {
    return std::make_unique<Event::Key>(*this);
}

Event::Key::Key(
    Event::Key::KeyBuilder& builder
) : AbstractData{"Key"} {
    fields.emplace_back(std::move(builder._time));
    fields.emplace_back(std::move(builder._press));
    fields.emplace_back(std::move(builder._release));
    fields.emplace_back(std::move(builder._repeat));
    fields.emplace_back(std::move(builder._character));
}