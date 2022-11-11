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

#ifndef EVGET_SCHEMAFIELD_H
#define EVGET_SCHEMAFIELD_H

#include <chrono>
#include <string_view>
#include <optional>
#include "DataType.h"
#include "ButtonAction.h"
#include "Device.h"
#include "evgetcore/Util.h"

namespace EvgetCore::Event::SchemaField {
    namespace detail {
        constexpr std::string_view ACTION_PRESS{"Press"};
        constexpr std::string_view ACTION_RELEASE{"Release"};
        constexpr std::string_view ACTION_REPEAT{"Repeat"};

        constexpr std::string_view DEVICE_TYPE_MOUSE{"Mouse"};
        constexpr std::string_view DEVICE_TYPE_KEYBOARD{"Keyboard"};
        constexpr std::string_view DEVICE_TYPE_TOUCHPAD{"Touchpad"};
        constexpr std::string_view DEVICE_TYPE_TOUCHSCREEN{"Touchscreen"};

        template<typename T>
        constexpr std::string
        optionalToString(std::optional<T> optional, EvgetCore::Util::Invocable<std::string, T> auto &&function) {
            if (!optional.has_value()) {
                return "";
            }
            return function(*optional);
        }
    }

    using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
    using Field = std::pair<std::string_view, DataType>;

    constexpr Field ACTION_FIELD{"Action", DataType::String};
    constexpr Field CHARACTER_FIELD{"Character", DataType::String};
    constexpr Field IDENTIFIER_FIELD{"Identifier", DataType::Integer};
    constexpr Field NAME_FIELD{"Name", DataType::String};
    constexpr Field TIMESTAMP_FIELD{"Timestamp", DataType::Timestamp};
    constexpr Field DEVICE_TYPE_FIELD{"DeviceType", DataType::String};
    constexpr Field INTERVAL_FIELD{"Interval", DataType::Interval};
    constexpr Field POSITIONX_FIELD{"PositionX", DataType::Double};
    constexpr Field POSITIONY_FIELD{"PositionY", DataType::Double};
    constexpr Field SCROLLDOWN_FIELD{"ScrollDown", DataType::Double};
    constexpr Field SCROLLLEFT_FIELD{"ScrollLeft", DataType::Double};
    constexpr Field SCROLLRIGHT_FIELD{"ScrollRight", DataType::Double};
    constexpr Field SCROLLUP_FIELD{"ScrollUp", DataType::Double};

    /**
     * Create a string from a string value.
     */
    constexpr std::string fromString(std::optional<std::string> value) {
        return detail::optionalToString(std::move(value), [](auto value){ return value; });
    }

    /**
     * Format a string from an int value.
     */
    constexpr std::string fromInt(std::optional<int> value) {
        return detail::optionalToString(value, [](auto value){ return std::to_string(value); });
    }

    /**
     * Format a string from a `Timestamp` value.
     */
    constexpr std::string fromTimestamp(std::optional<Timestamp> value) {
        return detail::optionalToString(value, [](auto value){
            std::stringstream stream{};
            stream << date::format("%FT%T%z", value);
            return stream.str();
        });
    }

    /**
     * Format a string from a `ButtonAction` value.
     */
    constexpr std::string fromButtonAction(std::optional<ButtonAction> value) {
        return detail::optionalToString(value, [](auto value){
            switch (value) {
                case ButtonAction::Press:
                    return std::string{detail::ACTION_PRESS};
                case ButtonAction::Release:
                    return std::string{detail::ACTION_RELEASE};
                case ButtonAction::Repeat:
                    return std::string{detail::ACTION_REPEAT};
            }
        });
    }

    /**
     * Format a string from a `Device` value.
     */
    constexpr std::string fromDevice(std::optional<Device> value) {
        return detail::optionalToString(value, [](auto value) {
            switch (value) {
                case Device::Mouse:
                    return std::string{detail::DEVICE_TYPE_MOUSE};
                case Device::Keyboard:
                    return std::string{detail::DEVICE_TYPE_KEYBOARD};
                case Device::Touchpad:
                    return std::string{detail::DEVICE_TYPE_TOUCHPAD};
                case Device::Touchscreen:
                    return std::string{detail::DEVICE_TYPE_TOUCHSCREEN};
            }
        });
    }

    /**
     * Format a string from a nanoseconds value.
     */
    constexpr std::string fromNanoseconds(std::optional<std::chrono::nanoseconds> value) {
        return detail::optionalToString(value, [](auto value){ return std::to_string(value.count()); });
    }

    /**
     * Format a string from a double value.
     */
    constexpr std::string fromDouble(std::optional<double> value) {
        return detail::optionalToString(value, [](auto value){ return std::to_string(value); });
    }
}

#endif //EVGET_SCHEMAFIELD_H
