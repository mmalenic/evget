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

#include <date/tz.h>

#include <array>
#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ButtonAction.h"
#include "Device.h"
#include "Graph.h"
#include "ModifierValue.h"
#include "evgetcore/Util.h"

namespace EvgetCore::Event {

namespace detail {
constexpr std::string_view ACTION_PRESS{"Press"};
constexpr std::string_view ACTION_RELEASE{"Release"};
constexpr std::string_view ACTION_REPEAT{"Repeat"};

constexpr std::string_view DEVICE_TYPE_MOUSE{"Mouse"};
constexpr std::string_view DEVICE_TYPE_KEYBOARD{"Keyboard"};
constexpr std::string_view DEVICE_TYPE_TOUCHPAD{"Touchpad"};
constexpr std::string_view DEVICE_TYPE_TOUCHSCREEN{"Touchscreen"};

constexpr std::string_view MODIFIER_VALUE_SHIFT{"Shift"};
constexpr std::string_view MODIFIER_VALUE_CAPSLOCK{"CapsLock"};
constexpr std::string_view MODIFIER_VALUE_CONTROL{"Control"};
constexpr std::string_view MODIFIER_VALUE_ALT{"Alt"};
constexpr std::string_view MODIFIER_VALUE_NUMLOCK{"NumLock"};
constexpr std::string_view MODIFIER_VALUE_MOD3{"Mod3"};
constexpr std::string_view MODIFIER_VALUE_SUPER{"Super"};
constexpr std::string_view MODIFIER_VALUE_MOD5{"Mod5"};

template <typename T>
constexpr std::string
optionalToString(std::optional<T> optional, EvgetCore::Util::Invocable<std::string, T> auto&& function) {
    if (!optional.has_value()) {
        return "";
    }
    return function(*optional);
}
}  // namespace detail

/**
 * Valid schema data types.
 */
enum class DataType { String, Integer, Timestamp, Interval, Double };

/**
 * A relation between edges in a graph.
 */
enum class Relation { OneToOne, OneToMany, ManyToOne, ManyToMany };

using Interval = std::chrono::microseconds;
using Timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
using FieldDefinition = std::pair<std::string_view, DataType>;

using Fields = std::vector<std::string>;

/**
 * Data contains the actual entries for the storage component. This is defined as a graph with a vector of strings
 * as node data, which represent the field entries for that node.
 */
using Data = Graph<Fields>;

/**
 * A Schema defines the shape of a `Data` entry. This tells the storage component how to store the `Data`.
 * This is defined as a graph with `FieldDefinition` as node data, and edge data indicating the `Relation`.
 */
using Schema = Graph<FieldDefinition, Relation>;

constexpr FieldDefinition ACTION_FIELD{"Action", DataType::String};
constexpr FieldDefinition CHARACTER_FIELD{"Character", DataType::String};
constexpr FieldDefinition IDENTIFIER_FIELD{"Identifier", DataType::Integer};
constexpr FieldDefinition NAME_FIELD{"Name", DataType::String};
constexpr FieldDefinition TIMESTAMP_FIELD{"Timestamp", DataType::Timestamp};
constexpr FieldDefinition DEVICE_TYPE_FIELD{"DeviceType", DataType::String};
constexpr FieldDefinition INTERVAL_FIELD{"Interval", DataType::Interval};
constexpr FieldDefinition POSITIONX_FIELD{"PositionX", DataType::Double};
constexpr FieldDefinition POSITIONY_FIELD{"PositionY", DataType::Double};
constexpr FieldDefinition SCROLLDOWN_FIELD{"ScrollDown", DataType::Double};
constexpr FieldDefinition SCROLLLEFT_FIELD{"ScrollLeft", DataType::Double};
constexpr FieldDefinition SCROLLRIGHT_FIELD{"ScrollRight", DataType::Double};
constexpr FieldDefinition SCROLLUP_FIELD{"ScrollUp", DataType::Double};
constexpr FieldDefinition MODIFIER_FIELD{"Modifier", DataType::String};
constexpr FieldDefinition DEVICE_NAME_FIELD{"DeviceName", DataType::String};
constexpr FieldDefinition FOCUS_WINDOW_NAME_FIELD{"FocusWindowName", DataType::String};
constexpr FieldDefinition FOCUS_WINDOW_POSITION_X_FIELD{"FocusWindowPositionX", DataType::Double};
constexpr FieldDefinition FOCUS_WINDOW_POSITION_Y_FIELD{"FocusWindowPositionY", DataType::Double};
constexpr FieldDefinition FOCUS_WINDOW_WIDTH_FIELD{"FocusWindowWidth", DataType::Double};
constexpr FieldDefinition FOCUS_WINDOW_HEIGHT_FIELD{"FocusWindowHeight", DataType::Double};

/**
 * Get the name of the FieldDefinition.
 */
constexpr std::string_view getName(FieldDefinition field) {
    return field.first;
}

/**
 * Create a string from a string value.
 */
constexpr std::string fromString(std::optional<std::string> value) {
    return detail::optionalToString(std::move(value), [](auto value) { return value; });
}

/**
 * Format a string from an int value.
 */
constexpr std::string fromInt(std::optional<int> value) {
    return detail::optionalToString(value, [](auto value) { return std::to_string(value); });
}

/**
 * Format a string from a `Timestamp` value.
 */
constexpr std::string fromTimestamp(std::optional<Timestamp> value) {
    return detail::optionalToString(value, [](auto value) {
        std::stringstream stream{};
        stream << date::format("%FT%T%z", value);
        return stream.str();
    });
}

/**
 * Format a string from a `ButtonAction` value.
 */
constexpr std::string fromButtonAction(std::optional<ButtonAction> value) {
    return detail::optionalToString(value, [](auto value) {
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
 * Format a string from a `ModifierValue`.
 */
constexpr std::string fromModifierValue(std::optional<ModifierValue> value) {
    return detail::optionalToString(value, [](auto value) {
        switch (value) {
            case ModifierValue::Shift:
                return std::string{detail::MODIFIER_VALUE_SHIFT};
            case ModifierValue::CapsLock:
                return std::string{detail::MODIFIER_VALUE_CAPSLOCK};
            case ModifierValue::Control:
                return std::string{detail::MODIFIER_VALUE_CONTROL};
            case ModifierValue::Alt:
                return std::string{detail::ACTION_PRESS};
            case ModifierValue::NumLock:
                return std::string{detail::MODIFIER_VALUE_NUMLOCK};
            case ModifierValue::Mod3:
                return std::string{detail::MODIFIER_VALUE_MOD3};
            case ModifierValue::Super:
                return std::string{detail::MODIFIER_VALUE_SUPER};
            case ModifierValue::Mod5:
                return std::string{detail::MODIFIER_VALUE_MOD5};
        }
    });
}

/**
 * Format a string from an `Interval` value.
 */
constexpr std::string fromInterval(std::optional<Interval> value) {
    return detail::optionalToString(value, [](auto value) { return std::to_string(value.count()); });
}

/**
 * Format a string from a double value.
 */
constexpr std::string fromDouble(std::optional<double> value) {
    return detail::optionalToString(value, [](auto value) { return std::to_string(value); });
}
}  // namespace EvgetCore::Event

#endif  // EVGET_SCHEMA_H
