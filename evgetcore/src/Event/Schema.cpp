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
//

#include <utility>
#include <date/tz.h>

#include "evgetcore/Event/Schema.h"

std::string EvgetCore::Event::Schema::fromString(std::optional<std::string> value) {
    return optionalToString(std::move(value), [](auto value){ return value; });
}

std::string EvgetCore::Event::Schema::fromInt(std::optional<int> value) {
    return optionalToString(value, [](auto value){ return std::to_string(value); });
}

std::string EvgetCore::Event::Schema::fromDateTime(std::optional<DateTime> value) {
    return optionalToString(value, [](auto value){
        std::stringstream stream{};
        stream << date::make_zoned(date::current_zone(), value);
        return stream.str();
    });
}

std::string EvgetCore::Event::Schema::fromButtonAction(std::optional<ButtonAction> value) {
    return optionalToString(value, [](auto value){
        switch (value) {
            case ButtonAction::Press:
                return std::string{ACTION_PRESS};
            case ButtonAction::Release:
                return std::string{ACTION_RELEASE};
            case ButtonAction::Repeat:
                return std::string{ACTION_REPEAT};
        }
    });
}

std::string EvgetCore::Event::Schema::fromDevice(std::optional<Device> value) {
    return optionalToString(value, [](auto value){
        switch (value) {
            case Device::Mouse:
                return std::string{DEVICE_TYPE_MOUSE};
            case Device::Keyboard:
                return std::string{DEVICE_TYPE_KEYBOARD};
            case Device::Touchpad:
                return std::string{DEVICE_TYPE_TOUCHPAD};
            case Device::Touchscreen:
                return std::string{DEVICE_TYPE_TOUCHSCREEN};
        }
    });
}

std::string EvgetCore::Event::Schema::fromNanoseconds(std::optional<std::chrono::nanoseconds> value) {
    return optionalToString(value, [](auto value){ return std::to_string(value.count()); });
}

std::string EvgetCore::Event::Schema::fromDouble(std::optional<double> value) {
    return optionalToString(value, [](auto value){ return std::to_string(value); });
}
