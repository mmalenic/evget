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

#ifndef EVGET_SRC_EVENT_FIELD_H
#define EVGET_SRC_EVENT_FIELD_H

#include <chrono>
#include <memory>
#include <variant>
#include <optional>
#include <vector>
#include <string>
#include "ButtonAction.h"
#include "Device.h"
#include "Direction.h"

namespace EvgetCore::Event {
    class Data;

    /**
     * A concept requiring the to_string function.
     */
    template<typename T>
    concept ToString = requires(T value) {
        { std::to_string(value) } -> std::convertible_to<std::string>;
    };

    /**
     * Represents a field in an event.
     */
    class Field {
    public:
        using DateTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;

        Field() = default;
        Field(std::string name, std::string entry);

        /**
         * Create a Field with an empty entry.
         */
        explicit Field(std::string name);

        /**
         * Get the entry.
         */
        [[nodiscard]] std::string getEntry() const;

        /**
         * Get the name.
         */
        [[nodiscard]] std::string getName() const;

        /**
         * Create an Action based on the ButtonAction enum.
         */
        static Field createAction(std::optional<ButtonAction> action);

        /**
         * Create a character from a string.
         */
        static Field createCharacter(std::optional<std::string> character);

        /**
         * Create an identifier using int id, usually representing a button id.
         */
        static Field createIdentifier(std::optional<int> id);

        /**
         * Create a name field.
         */
        static Field createName(std::optional<std::string> name);

        /**
         * Create a date time field using a DateTime value.
         */
        static Field createDateTime(std::optional<DateTime> dateTime);

        /**
         * Create a date time field using a DateTime value.
         */
        static Field createDeviceType(std::optional<Device> device);

        /**
         * Create a time field, which represents an interval of time.
         */
        static Field createTime(std::optional<std::chrono::nanoseconds> interval);

        /**
         * Create a position x Field.
         */
        static Field createPositionX(std::optional<double> position);

        /**
         * Create a position y Field.
         */
        static Field createPositionY(std::optional<double> position);

        /**
         * Create a scroll Field using the Direction and amount.
         */
        static Field createScroll(Direction direction, std::optional<double> amount);

    private:
        static constexpr std::string_view ACTION_FIELD_NAME{"Action"};
        static constexpr std::string_view ACTION_PRESS{"Press"};
        static constexpr std::string_view ACTION_RELEASE{"Release"};
        static constexpr std::string_view ACTION_REPEAT{"Repeat"};
        static constexpr std::string_view CHARACTER_FIELD_NAME{"Character"};
        static constexpr std::string_view IDENTIFIER_FIELD_NAME{"Identifier"};
        static constexpr std::string_view NAME_FIELD_NAME{"Name"};
        static constexpr std::string_view DATE_TIME_FIELD_NAME{"DateTime"};
        static constexpr std::string_view DEVICE_TYPE_FIELD_NAME{"DeviceType"};
        static constexpr std::string_view DEVICE_TYPE_MOUSE{"Mouse"};
        static constexpr std::string_view DEVICE_TYPE_KEYBOARD{"Keyboard"};
        static constexpr std::string_view DEVICE_TYPE_TOUCHPAD{"Touchpad"};
        static constexpr std::string_view DEVICE_TYPE_TOUCHSCREEN{"Touchscreen"};
        static constexpr std::string_view TIME_FIELD_NAME{"Time"};
        static constexpr std::string_view POSITIONX_FIELD_NAME{"PositionX"};
        static constexpr std::string_view POSITIONY_FIELD_NAME{"PositionY"};
        static constexpr std::string_view SCROLLDOWN_FIELD_NAME{"ScrollDown"};
        static constexpr std::string_view SCROLLLEFT_FIELD_NAME{"ScrollLeft"};
        static constexpr std::string_view SCROLLRIGHT_FIELD_NAME{"ScrollRight"};
        static constexpr std::string_view SCROLLUP_FIELD_NAME{"ScrollUp"};

        /**
         * Create a Field using a string value.
         */
        static Field createOptionalString(std::string_view name, std::optional<std::string> value);

        /**
         * Create a Field using a generic optional value with a std::to_string call.
         */
        template<ToString T>
        static Field createOptionalToString(std::string_view name, std::optional<T> value);

        std::string name{};
        std::string entry{};
    };

    template<ToString T>
    EvgetCore::Event::Field
    EvgetCore::Event::Field::createOptionalToString(std::string_view name, std::optional<T> value) {
        if (!value.has_value()) {
            return Field{std::string{name}};
        }
        return {std::string{name}, std::to_string(*value)};
    }

}

#endif //EVGET_SRC_EVENT_FIELD_H
