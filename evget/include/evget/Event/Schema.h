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

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "evget/Error.h"
#include "evget/Event/ButtonAction.h"
#include "evget/Event/Device.h"
#include "evget/Event/Entry.h"
#include "evget/Event/ModifierValue.h"

namespace evget::Event {

namespace detail {
constexpr std::string_view ACTION_PRESS{"Press"};
constexpr std::string_view ACTION_RELEASE{"Release"};
constexpr std::string_view ACTION_REPEAT{"Repeat"};

constexpr std::string_view DEVICE_TYPE_MOUSE{"Mouse"};
constexpr std::string_view DEVICE_TYPE_KEYBOARD{"Keyboard"};
constexpr std::string_view DEVICE_TYPE_TOUCHPAD{"Touchpad"};
constexpr std::string_view DEVICE_TYPE_TOUCHSCREEN{"Touchscreen"};
constexpr std::string_view DEVICE_TYPE_UNKNOWN{"Unknown"};

constexpr std::string_view MODIFIER_VALUE_SHIFT{"Shift"};
constexpr std::string_view MODIFIER_VALUE_CAPSLOCK{"CapsLock"};
constexpr std::string_view MODIFIER_VALUE_CONTROL{"Control"};
constexpr std::string_view MODIFIER_VALUE_ALT{"Alt"};
constexpr std::string_view MODIFIER_VALUE_NUMLOCK{"NumLock"};
constexpr std::string_view MODIFIER_VALUE_MOD3{"Mod3"};
constexpr std::string_view MODIFIER_VALUE_SUPER{"Super"};
constexpr std::string_view MODIFIER_VALUE_MOD5{"Mod5"};

constexpr std::string_view DATA_TYPE_STRING{"String"};
constexpr std::string_view DATA_TYPE_INTEGER{"Integer"};
constexpr std::string_view DATA_TYPE_TIMESTAMP{"Timestamp"};
constexpr std::string_view DATA_TYPE_INTERVAL{"Interval"};
constexpr std::string_view DATA_TYPE_DOUBLE{"Double"};

constexpr std::string_view RELATION_ONE_TO_ONE{"OneToOne"};
constexpr std::string_view RELATION_ONE_TO_MANY{"OneToMany"};
constexpr std::string_view RELATION_MANY_TO_ONE{"ManyToOne"};
constexpr std::string_view RELATION_MANY_TO_MANY{"ManyToMany"};

constexpr std::string_view ENTRY_TYPE_KEY{"Key"};
constexpr std::string_view ENTRY_TYPE_MOUSE_MOVE{"MouseMove"};
constexpr std::string_view ENTRY_TYPE_MOUSE_CLICK{"MouseClick"};
constexpr std::string_view ENTRY_TYPE_MOUSE_SCROLL{"MouseScroll"};

template <typename T>
constexpr std::string optionalToString(std::optional<T> optional, Invocable<std::string, T> auto&& function) {
    if (!optional.has_value()) {
        return "";
    }
    return function(*optional);
}

}  // namespace detail

/**
 * Valid schema data types.
 */
enum class DataType : std::uint8_t { String, Integer, Timestamp, Interval, Double };

/**
 * A relation between edges in a graph.
 */
enum class Relation : std::uint8_t { OneToOne, OneToMany, ManyToOne, ManyToMany };

using Interval = std::chrono::microseconds;
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

using FieldDefinition = std::pair<std::string_view, DataType>;

/**
 * A field represents the name, data type and data of a field.
 */
struct Field {
    FieldDefinition fieldDefinition;
    std::string data;
};

using Fields = std::vector<Field>;

constexpr FieldDefinition ACTION_FIELD{"Action", DataType::String};
constexpr FieldDefinition CHARACTER_FIELD{"Character", DataType::String};
constexpr FieldDefinition IDENTIFIER_FIELD{"Identifier", DataType::Integer};
constexpr FieldDefinition NAME_FIELD{"Name", DataType::String};
constexpr FieldDefinition TIMESTAMP_FIELD{"Timestamp", DataType::Timestamp};
constexpr FieldDefinition DEVICE_TYPE_FIELD{"DeviceType", DataType::String};
constexpr FieldDefinition INTERVAL_FIELD{"Interval", DataType::Interval};
constexpr FieldDefinition POSITIONX_FIELD{"PositionX", DataType::Double};
constexpr FieldDefinition POSITIONY_FIELD{"PositionY", DataType::Double};
constexpr FieldDefinition SCROLL_VERTICAL_FIELD{"ScrollVertical", DataType::Double};
constexpr FieldDefinition SCROLL_HORIZONTAL_FIELD{"ScrollHorizontal", DataType::Double};
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
 * Get the type of the FieldDefinition.
 */
constexpr std::string_view getType(FieldDefinition field) {
    switch (field.second) {
        case DataType::String:
            return detail::DATA_TYPE_STRING;
        case DataType::Integer:
            return detail::DATA_TYPE_INTEGER;
        case DataType::Timestamp:
            return detail::DATA_TYPE_TIMESTAMP;
        case DataType::Interval:
            return detail::DATA_TYPE_INTERVAL;
        case DataType::Double:
            return detail::DATA_TYPE_DOUBLE;
    }

    return "";
}

/**
 * Get the type of the FieldDefinition.
 */
constexpr std::string_view getRelation(Relation relation) {
    switch (relation) {
        case Relation::OneToOne:
            return detail::RELATION_ONE_TO_ONE;
        case Relation::OneToMany:
            return detail::RELATION_ONE_TO_MANY;
        case Relation::ManyToOne:
            return detail::RELATION_MANY_TO_ONE;
        case Relation::ManyToMany:
            return detail::RELATION_MANY_TO_MANY;
    }

    return "";
}

/**
 * Convert an enum to its underlying value as a string.
 */
template <class Enum>
constexpr std::string toUnderlyingOptional(std::optional<Enum> value) {
    return detail::optionalToString(value, [](auto value) { return toUnderlying(value); });
}

/**
 * Convert an enum to its underlying value as a string.
 */
template <class Enum>
constexpr std::string toUnderlying(Enum value) {
    return std::to_string(std::to_underlying(value));
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
constexpr std::string fromTimestamp(const std::optional<Timestamp> optional) {
    return detail::optionalToString(optional, [](auto value) {
        std::stringstream stream{};
        stream << date::format("%FT%T%z", value);
        return stream.str();
    });
}

/**
 * Format a string from a `ButtonAction` value.
 */
constexpr std::string fromButtonAction(const std::optional<ButtonAction> optional) {
    return detail::optionalToString(optional, [](auto value) {
        switch (value) {
            case ButtonAction::Press:
                return std::string{detail::ACTION_PRESS};
            case ButtonAction::Release:
                return std::string{detail::ACTION_RELEASE};
            case ButtonAction::Repeat:
                return std::string{detail::ACTION_REPEAT};
            default:
                return std::string{};
        }
    });
}

/**
 * Format a string from a `Device` value.
 */
constexpr std::string fromDevice(const std::optional<Device> optional) {
    return detail::optionalToString(optional, [](auto value) {
        switch (value) {
            case Device::Mouse:
                return std::string{detail::DEVICE_TYPE_MOUSE};
            case Device::Keyboard:
                return std::string{detail::DEVICE_TYPE_KEYBOARD};
            case Device::Touchpad:
                return std::string{detail::DEVICE_TYPE_TOUCHPAD};
            case Device::Touchscreen:
                return std::string{detail::DEVICE_TYPE_TOUCHSCREEN};
            case Device::Unknown:
                return std::string{detail::DEVICE_TYPE_UNKNOWN};
            default:
                return std::string{};
        }
    });
}

/**
 * Format a string from a `ModifierValue`.
 */
constexpr std::string fromModifierValue(const std::optional<ModifierValue> optional) {
    return detail::optionalToString(optional, [](auto value) {
        switch (value) {
            case ModifierValue::Shift:
                return std::string{detail::MODIFIER_VALUE_SHIFT};
            case ModifierValue::CapsLock:
                return std::string{detail::MODIFIER_VALUE_CAPSLOCK};
            case ModifierValue::Control:
                return std::string{detail::MODIFIER_VALUE_CONTROL};
            case ModifierValue::Alt:
                return std::string{detail::MODIFIER_VALUE_ALT};
            case ModifierValue::NumLock:
                return std::string{detail::MODIFIER_VALUE_NUMLOCK};
            case ModifierValue::Mod3:
                return std::string{detail::MODIFIER_VALUE_MOD3};
            case ModifierValue::Super:
                return std::string{detail::MODIFIER_VALUE_SUPER};
            case ModifierValue::Mod5:
                return std::string{detail::MODIFIER_VALUE_MOD5};
            default:
                return std::string{};
        }
    });
}

/**
 * Format a string from a `EntryType`.
 */
constexpr std::string fromEntryType(const std::optional<EntryType> optional) {
    return detail::optionalToString(optional, [](auto value) {
        switch (value) {
            case EntryType::Key:
                return std::string{detail::ENTRY_TYPE_KEY};
            case EntryType::MouseMove:
                return std::string{detail::ENTRY_TYPE_MOUSE_MOVE};
            case EntryType::MouseClick:
                return std::string{detail::ENTRY_TYPE_MOUSE_CLICK};
            case EntryType::MouseScroll:
                return std::string{detail::ENTRY_TYPE_MOUSE_SCROLL};
            default:
                return std::string{};
        }
    });
}

/**
 * Convert an integer string representation to an enum.
 */
template <class Enum>
constexpr std::optional<Enum> fromUnderlying(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    return std::optional{static_cast<Enum>(std::stoi(value))};
}

/**
 * Format a string from an `Interval` value.
 */
constexpr std::string fromInterval(const std::optional<Interval> optional) {
    return detail::optionalToString(optional, [](auto value) { return std::to_string(value.count()); });
}

/**
 * Format a string from a double value.
 */
constexpr std::string fromDouble(const std::optional<double> optional) {
    return detail::optionalToString(optional, [](auto value) { return std::to_string(value); });
}
}  // namespace evget::Event

#endif  // EVGET_SCHEMA_H
