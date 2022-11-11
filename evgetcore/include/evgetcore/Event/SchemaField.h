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

namespace EvgetCore::Event {
    class SchemaField {
    public:
        using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
        using Field = std::pair<std::string_view, DataType>;

        static constexpr Field ACTION_FIELD{"Action", DataType::String};
        static constexpr Field CHARACTER_FIELD{"Character", DataType::String};
        static constexpr Field IDENTIFIER_FIELD{"Identifier", DataType::Integer};
        static constexpr Field NAME_FIELD{"Name", DataType::String};
        static constexpr Field TIMESTAMP_FIELD{"Timestamp", DataType::Timestamp};
        static constexpr Field DEVICE_TYPE_FIELD{"DeviceType", DataType::String};
        static constexpr Field INTERVAL_FIELD{"Interval", DataType::Interval};
        static constexpr Field POSITIONX_FIELD{"PositionX", DataType::Double};
        static constexpr Field POSITIONY_FIELD{"PositionY", DataType::Double};
        static constexpr Field SCROLLDOWN_FIELD{"ScrollDown", DataType::Double};
        static constexpr Field SCROLLLEFT_FIELD{"ScrollLeft", DataType::Double};
        static constexpr Field SCROLLRIGHT_FIELD{"ScrollRight", DataType::Double};
        static constexpr Field SCROLLUP_FIELD{"ScrollUp", DataType::Double};

        /**
         * Create a string from a string value.
         */
        constexpr static std::string fromString(std::optional<std::string> value);

        /**
         * Format a string from an int value.
         */
        constexpr static std::string fromInt(std::optional<int> value);

        /**
         * Format a string from a `Timestamp` value.
         */
        constexpr static std::string fromDateTime(std::optional<Timestamp> value);

        /**
         * Format a string from a `ButtonAction` value.
         */
        constexpr static std::string fromButtonAction(std::optional<ButtonAction> value);

        /**
         * Format a string from a `Device` value.
         */
        constexpr static std::string fromDevice(std::optional<Device> value);

        /**
         * Format a string from a nanoseconds value.
         */
        constexpr static std::string fromNanoseconds(std::optional<std::chrono::nanoseconds> value);

        /**
         * Format a string from a double value.
         */
        constexpr static std::string fromDouble(std::optional<double> value);

    private:
        static constexpr std::string_view ACTION_PRESS{"Press"};
        static constexpr std::string_view ACTION_RELEASE{"Release"};
        static constexpr std::string_view ACTION_REPEAT{"Repeat"};

        static constexpr std::string_view DEVICE_TYPE_MOUSE{"Mouse"};
        static constexpr std::string_view DEVICE_TYPE_KEYBOARD{"Keyboard"};
        static constexpr std::string_view DEVICE_TYPE_TOUCHPAD{"Touchpad"};
        static constexpr std::string_view DEVICE_TYPE_TOUCHSCREEN{"Touchscreen"};

        template <typename T>
        constexpr static std::string optionalToString(std::optional<T> optional, EvgetCore::Util::Invocable<std::string, T> auto&& function);
    };

    template<typename T>
    constexpr std::string SchemaField::optionalToString(std::optional<T> optional,
                                                        EvgetCore::Util::Invocable<std::string, T> auto &&function) {
        if (!optional.has_value()) {
            return "";
        }
        return function(*optional);
    }

    constexpr std::string SchemaField::fromInt(std::optional<int> value) {
        return optionalToString(value, [](auto value){ return std::to_string(value); });
    }

    constexpr std::string SchemaField::fromDateTime(std::optional<Timestamp> value) {
        return optionalToString(value, [](auto value){
            std::stringstream stream{};
            stream << date::format("%FT%T%z", value);
            return stream.str();
        });
    }

    constexpr std::string SchemaField::fromString(std::optional<std::string> value) {
        return optionalToString(std::move(value), [](auto value){ return value; });
    }

    constexpr std::string SchemaField::fromButtonAction(std::optional<ButtonAction> value) {
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

    constexpr std::string SchemaField::fromDevice(std::optional<Device> value) {
        return optionalToString(value, [](auto value) {
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

    constexpr std::string SchemaField::fromNanoseconds(std::optional<std::chrono::nanoseconds> value) {
        return optionalToString(value, [](auto value){ return std::to_string(value.count()); });
    }

    constexpr std::string SchemaField::fromDouble(std::optional<double> value) {
        return optionalToString(value, [](auto value){ return std::to_string(value); });
    }
}

#endif //EVGET_SCHEMAFIELD_H
