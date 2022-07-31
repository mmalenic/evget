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

#include "evgetcore/Event/Field.h"
#include "evgetcore/Event/AbstractData.h"

#include <date/tz.h>
#include <utility>

EvgetCore::Event::Field::Field(std::string name) : name{std::move(name)}, entry{} {
}

EvgetCore::Event::Field::Field(std::string_view name) : name{name}, entry{} {
}

EvgetCore::Event::Field::Field(std::string name, std::string entry) : name{std::move(name)}, entry{std::move(entry)} {
}

EvgetCore::Event::Field::Field(std::string_view name, std::string_view entry) : name{name}, entry{entry} {
}

EvgetCore::Event::Field::Field(std::string_view name, std::string entry) : name{name}, entry{entry} {
}

EvgetCore::Event::Field::Field(std::string name, Entries entries) : name{std::move(name)}, entry{std::move(entries)} {
}

std::string EvgetCore::Event::Field::getEntry() const {
    return std::get<std::string>(entry);
}

std::string EvgetCore::Event::Field::getName() const {
    return name;
}

EvgetCore::Event::Field::Iterator EvgetCore::Event::Field::begin() const {
    return std::get<Entries>(entry).begin();
}

EvgetCore::Event::Field::Iterator EvgetCore::Event::Field::end() const {
    return std::get<Entries>(entry).end();
}

EvgetCore::Event::Field EvgetCore::Event::Field::createAction(std::optional<EvgetCore::Event::ButtonAction> action) {
    if (!action.has_value()) {
        return Field{ACTION_FIELD_NAME};
    }
    switch (*action) {
        case ButtonAction::Press:
            return {ACTION_FIELD_NAME, ACTION_PRESS};
        case ButtonAction::Release:
            return {ACTION_FIELD_NAME, ACTION_RELEASE};
        case ButtonAction::Repeat:
            return {ACTION_FIELD_NAME, ACTION_REPEAT};
    }
}

EvgetCore::Event::Field EvgetCore::Event::Field::createCharacter(std::optional<std::string> character) {
    return {CHARACTER_FIELD_NAME, std::move(character)};
}

EvgetCore::Event::Field EvgetCore::Event::Field::createIdentifier(std::optional<int> id) {
    return {IDENTIFIER_FIELD_NAME, std::to_string(id)};
}

EvgetCore::Event::Field EvgetCore::Event::Field::createName(std::optional<std::string> name) {
    return {NAME_FIELD_NAME, std::move(name)};
}

EvgetCore::Event::Field EvgetCore::Event::Field::createDateTime(std::optional<EvgetCore::Event::Field::DateTime> dateTime) {
    if (!dateTime.has_value()) {
        return Field{DATE_TIME_FIELD_NAME};
    }

    std::stringstream stream{};
    stream << date::make_zoned(date::current_zone(), dateTime);
    return {DATE_TIME_FIELD_NAME, stream.str()};
}

EvgetCore::Event::Field EvgetCore::Event::Field::createDeviceType(std::optional<EvgetCore::Event::Device> device) {
    if (!device.has_value()) {
        return Field{DEVICE_TYPE_FIELD_NAME};
    }

    switch (*device) {
        case Device::Mouse:
            return {DEVICE_TYPE_FIELD_NAME, DEVICE_TYPE_MOUSE};
        case Device::Keyboard:
            return {DEVICE_TYPE_FIELD_NAME, DEVICE_TYPE_KEYBOARD};
        case Device::Touchpad:
            return {DEVICE_TYPE_FIELD_NAME, DEVICE_TYPE_TOUCHPAD};
        case Device::Touchscreen:
            return {DEVICE_TYPE_FIELD_NAME, DEVICE_TYPE_TOUCHSCREEN};
    }
}

EvgetCore::Event::Field EvgetCore::Event::Field::createTime(std::optional<std::chrono::nanoseconds> interval) {
    return {TIME_FIELD_NAME, std::to_string(interval.count())};
}

EvgetCore::Event::Field EvgetCore::Event::Field::createPositionX(std::optional<double> position) {
    return createDouble(POSITIONX_FIELD_NAME, position);
}

EvgetCore::Event::Field EvgetCore::Event::Field::createPositionY(std::optional<double> position) {
    return createDouble(POSITIONY_FIELD_NAME, position);
}

EvgetCore::Event::Field EvgetCore::Event::Field::createScroll(EvgetCore::Event::Direction direction, std::optional<double> amount) {
    switch (direction) {
        case Direction::Down:
            return createDouble(SCROLLDOWN_FIELD_NAME, amount);
        case Direction::Left:
            return createDouble(SCROLLLEFT_FIELD_NAME, amount);
        case Direction::Right:
            return createDouble(SCROLLRIGHT_FIELD_NAME, amount);
        case Direction::Up:
            return createDouble(SCROLLUP_FIELD_NAME,amount);
    }
}

EvgetCore::Event::Field EvgetCore::Event::Field::createDouble(std::string_view name, double value) {
    return {name, std::to_string(value)};
}
