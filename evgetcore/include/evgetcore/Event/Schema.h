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

#ifndef EVGET_SCHEMA_H
#define EVGET_SCHEMA_H

#include <map>
#include <string>
#include <optional>
#include <chrono>
#include <array>
#include <utility>
#include <date/tz.h>
#include "ButtonAction.h"
#include "Device.h"
#include "evgetcore/Util.h"

namespace EvgetCore::Event {
    /**
     * A Schema defines the shape of a `Data` entry. This tells the storage component how to store the `Data`.
     */
    template<std::size_t NFields = 0, std::size_t NLinkedTo = 0, std::size_t NUniquelyLinkedTo = 0>
    class Schema {
    public:
        static constexpr std::string_view ACTION_FIELD_NAME{"Action"};
        static constexpr std::string_view CHARACTER_FIELD_NAME{"Character"};
        static constexpr std::string_view IDENTIFIER_FIELD_NAME{"Identifier"};
        static constexpr std::string_view NAME_FIELD_NAME{"Name"};
        static constexpr std::string_view DATE_TIME_FIELD_NAME{"DateTime"};
        static constexpr std::string_view DEVICE_TYPE_FIELD_NAME{"DeviceType"};
        static constexpr std::string_view TIME_FIELD_NAME{"Time"};
        static constexpr std::string_view POSITIONX_FIELD_NAME{"PositionX"};
        static constexpr std::string_view POSITIONY_FIELD_NAME{"PositionY"};
        static constexpr std::string_view SCROLLDOWN_FIELD_NAME{"ScrollDown"};
        static constexpr std::string_view SCROLLLEFT_FIELD_NAME{"ScrollLeft"};
        static constexpr std::string_view SCROLLRIGHT_FIELD_NAME{"ScrollRight"};
        static constexpr std::string_view SCROLLUP_FIELD_NAME{"ScrollUp"};

        using DateTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
        using Field = std::pair<std::string_view, std::string_view>;

        constexpr Schema(std::string_view name, std::array<Field, NFields> fields = {}, std::array<Schema, NLinkedTo> linkedTo = {}, std::array<Schema, NUniquelyLinkedTo> uniquelyLinkedTo = {});

        /**
         * Get the fields in this Schema.
         */
        [[nodiscard]] constexpr const std::array<Field, NFields> &getFields() const;

        /**
         * Get the linked to schemas.
         */
        [[nodiscard]] constexpr const std::array<Schema, NLinkedTo> &getLinkedTo() const;

        /**
         * Get the uniquely linked to schemas.
         */
        [[nodiscard]] constexpr const std::array<Schema, NUniquelyLinkedTo> &getUniquelyLinkedTo() const;

        /**
         * Create a string from a string value.
         */
        constexpr static std::string fromString(std::optional<std::string> value);

        /**
         * Format a string from an int value.
         */
        constexpr static std::string fromInt(std::optional<int> value);

        /**
         * Format a string from a `DateTime` value.
         */
        constexpr static std::string fromDateTime(std::optional<DateTime> value);

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

        std::string_view name{};
        std::array<Field, NFields> fields{};
        std::array<Schema, NLinkedTo> linkedTo{};
        std::array<Schema, NUniquelyLinkedTo> uniquelyLinkedTo{};
    };

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    template<typename T>
    constexpr std::string Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::optionalToString(std::optional<T> optional,
                                                                                          EvgetCore::Util::Invocable<std::string, T> auto &&function) {
        if (!optional.has_value()) {
            return "";
        }
        return function(*optional);
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::Schema(std::string_view name, std::array<Field, NFields> fields,
                                                                    std::array<Schema, NLinkedTo> linkedTo,
                                                                    std::array<Schema, NUniquelyLinkedTo> uniquelyLinkedTo):
                                                                    name{name},
                                                                    fields{std::move(fields)},
                                                                    linkedTo{std::move(linkedTo)},
                                                                    uniquelyLinkedTo{std::move(uniquelyLinkedTo)} {
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr const std::array<typename Schema<>::Field, NFields> &Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::getFields() const {
        return fields;
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr const std::array<Schema<NFields, NLinkedTo, NUniquelyLinkedTo>, NLinkedTo> &Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::getLinkedTo() const {
        return linkedTo;
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr const std::array<Schema<NFields, NLinkedTo, NUniquelyLinkedTo>, NUniquelyLinkedTo> &Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::getUniquelyLinkedTo() const {
        return uniquelyLinkedTo;
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr std::string Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::fromInt(std::optional<int> value) {
        return optionalToString(value, [](auto value){ return std::to_string(value); });
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr std::string Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::fromDateTime(std::optional<DateTime> value) {
        return optionalToString(value, [](auto value){
            std::stringstream stream{};
            stream << date::format("%FT%T%z", value);
            return stream.str();
        });
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr std::string Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::fromString(std::optional<std::string> value) {
        return optionalToString(std::move(value), [](auto value){ return value; });
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr std::string
    Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::fromButtonAction(std::optional<ButtonAction> value) {
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

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr std::string Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::fromDevice(std::optional<Device> value) {
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

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr std::string
    Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::fromNanoseconds(std::optional<std::chrono::nanoseconds> value) {
        return optionalToString(value, [](auto value){ return std::to_string(value.count()); });
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr std::string Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::fromDouble(std::optional<double> value) {
        return optionalToString(value, [](auto value){ return std::to_string(value); });
    }
}


#endif //EVGET_SCHEMA_H
