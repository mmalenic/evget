/**
 * \file schema.h
 * \brief Event schema definitions and utility functions for data type conversion.
 * 
 * This file contains the core schema definitions for event data types,
 * field definitions, and utility functions for converting between different
 * data representations used in the event system.
 */

#ifndef EVGET_EVENT_SCHEMA_H
#define EVGET_EVENT_SCHEMA_H

#include <chrono>
#include <cstdint>
#include <format>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "evget/error.h"
#include "evget/event/button_action.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/modifier_value.h"

namespace evget {

namespace detail {
constexpr std::string_view kActionPress{"Press"};
constexpr std::string_view kActionRelease{"Release"};
constexpr std::string_view kActionRepeat{"Repeat"};

constexpr std::string_view kDeviceTypeMouse{"Mouse"};
constexpr std::string_view kDeviceTypeKeyboard{"Keyboard"};
constexpr std::string_view kDeviceTypeTouchpad{"Touchpad"};
constexpr std::string_view kDeviceTypeTouchscreen{"Touchscreen"};
constexpr std::string_view kDeviceTypeUnknown{"Unknown"};

constexpr std::string_view kModifierValueShift{"Shift"};
constexpr std::string_view kModifierValueCapslock{"CapsLock"};
constexpr std::string_view kModifierValueControl{"Control"};
constexpr std::string_view kModifierValueAlt{"Alt"};
constexpr std::string_view kModifierValueNumlock{"NumLock"};
constexpr std::string_view kModifierValueMoD3{"Mod3"};
constexpr std::string_view kModifierValueSuper{"Super"};
constexpr std::string_view kModifierValueMoD5{"Mod5"};

constexpr std::string_view kDataTypeString{"String"};
constexpr std::string_view kDataTypeInteger{"Integer"};
constexpr std::string_view kDataTypeTimestamp{"Timestamp"};
constexpr std::string_view kDataTypeInterval{"Interval"};
constexpr std::string_view kDataTypeDouble{"Double"};

constexpr std::string_view kRelationOneToOne{"OneToOne"};
constexpr std::string_view kRelationOneToMany{"OneToMany"};
constexpr std::string_view kRelationManyToOne{"ManyToOne"};
constexpr std::string_view kRelationManyToMany{"ManyToMany"};

constexpr std::string_view kEntryTypeKey{"Key"};
constexpr std::string_view kEntryTypeMouseMove{"MouseMove"};
constexpr std::string_view kEntryTypeMouseClick{"MouseClick"};
constexpr std::string_view kEntryTypeMouseScroll{"MouseScroll"};

template <typename T>
constexpr std::string OptionalToString(std::optional<T> optional, Invocable<std::string, T> auto&& function) {
    if (!optional.has_value()) {
        return "";
    }
    return function(*optional);
}

} // namespace detail

/**
 * Valid schema data types.
 */
enum class DataType : std::uint8_t {
    kString,     ///< A string type
    kInteger,    ///< An integer type
    kTimestamp,  ///< A timestamp type
    kInterval,   ///< An interval type
    kDouble      ///< A double type
};

using IntervalType = std::chrono::microseconds;
using TimestampType = std::chrono::time_point<std::chrono::system_clock>;

using FieldDefinition = std::pair<std::string_view, DataType>;

/**
 * \brief A field represents the name, data type, and data of a field.
 */
struct Field {
    FieldDefinition field_definition;  ///< Field definition
    std::string data;                  ///< Actual data value as string
};

using Fields = std::vector<Field>;

constexpr FieldDefinition kActionField{"Action", DataType::kString};
constexpr FieldDefinition kCharacterField{"Character", DataType::kString};
constexpr FieldDefinition kIdentifierField{"Identifier", DataType::kInteger};
constexpr FieldDefinition kNameField{"Name", DataType::kString};
constexpr FieldDefinition kTimestampField{"Timestamp", DataType::kTimestamp};
constexpr FieldDefinition kDeviceTypeField{"DeviceType", DataType::kString};
constexpr FieldDefinition kIntervalField{"Interval", DataType::kInterval};
constexpr FieldDefinition kPositionxField{"PositionX", DataType::kDouble};
constexpr FieldDefinition kPositionyField{"PositionY", DataType::kDouble};
constexpr FieldDefinition kScrollVerticalField{"ScrollVertical", DataType::kDouble};
constexpr FieldDefinition kScrollHorizontalField{"ScrollHorizontal", DataType::kDouble};
constexpr FieldDefinition kScrollrightField{"ScrollRight", DataType::kDouble};
constexpr FieldDefinition kScrollupField{"ScrollUp", DataType::kDouble};
constexpr FieldDefinition kModifierField{"Modifier", DataType::kString};
constexpr FieldDefinition kDeviceNameField{"DeviceName", DataType::kString};
constexpr FieldDefinition kFocusWindowNameField{"FocusWindowName", DataType::kString};
constexpr FieldDefinition kFocusWindowPositionXField{"FocusWindowPositionX", DataType::kDouble};
constexpr FieldDefinition kFocusWindowPositionYField{"FocusWindowPositionY", DataType::kDouble};
constexpr FieldDefinition kFocusWindowWidthField{"FocusWindowWidth", DataType::kDouble};
constexpr FieldDefinition kFocusWindowHeightField{"FocusWindowHeight", DataType::kDouble};

/**
 * \brief Get the name of the `FieldDefinition`.
 * \param field field definition to get name from
 * \return string view of the field name
 */
constexpr std::string_view GetName(FieldDefinition field) {
    return field.first;
}

/**
 * \brief Get the type of the `FieldDefinition` as a string.
 * \param field field definition to get type from
 * \return string view of the field type
 */
constexpr std::string_view GetType(FieldDefinition field) {
    switch (field.second) {
        case DataType::kString:
            return detail::kDataTypeString;
        case DataType::kInteger:
            return detail::kDataTypeInteger;
        case DataType::kTimestamp:
            return detail::kDataTypeTimestamp;
        case DataType::kInterval:
            return detail::kDataTypeInterval;
        case DataType::kDouble:
            return detail::kDataTypeDouble;
    }

    return "";
}

/**
 * \brief Convert an optional enum to its underlying value as a string.
 * \tparam Enum enum type
 * \param value optional enum value
 * \return string representation of the underlying value, or empty string if `nullopt`
 */
template <class Enum>
constexpr std::string ToUnderlyingOptional(std::optional<Enum> value) {
    return detail::OptionalToString(value, [](auto value) { return ToUnderlying(value); });
}

/**
 * \brief Convert an enum to its underlying value as a string.
 * \tparam Enum enum type
 * \param value enum value
 * \return string representation of the underlying value
 */
template <class Enum>
constexpr std::string ToUnderlying(Enum value) {
    return std::to_string(std::to_underlying(value));
}

/**
 * \brief Create a string from an optional string value.
 * \param value optional string value
 * \return string value or empty string if `nullopt`
 */
constexpr std::string FromString(std::optional<std::string> value) {
    return detail::OptionalToString(std::move(value), [](auto value) { return value; });
}

/**
 * \brief Format a string from an optional int value.
 * \param value optional integer value
 * \return string representation of the integer, or empty string if `nullopt`
 */
constexpr std::string FromInt(std::optional<int> value) {
    return detail::OptionalToString(value, [](auto value) { return std::to_string(value); });
}

/**
 * \brief Format a string from an optional `Timestamp` value.
 * \param optional optional timestamp value
 * \return ISO 8601 formatted timestamp string, or empty string if `nullopt`
 */
constexpr std::string FromTimestamp(const std::optional<TimestampType> optional) {
    return detail::OptionalToString(optional, [](auto value) {
        std::stringstream stream{};
        stream << std::format("{:%Y-%m-%dT%H:%M:%S%z}", value);
        return stream.str();
    });
}

/**
 * \brief Format a string from an optional `ButtonAction` value.
 * \param optional optional button action value
 * \return string representation of the button action, or empty string if `nullopt`
 */
constexpr std::string FromButtonAction(const std::optional<ButtonAction> optional) {
    return detail::OptionalToString(optional, [](auto value) {
        switch (value) {
            case ButtonAction::kPress:
                return std::string{detail::kActionPress};
            case ButtonAction::kRelease:
                return std::string{detail::kActionRelease};
            case ButtonAction::kRepeat:
                return std::string{detail::kActionRepeat};
            default:
                return std::string{};
        }
    });
}

/**
 * \brief Format a string from an optional `DeviceType` value.
 * \param optional optional device type value
 * \return string representation of the device type, or empty string if `nullopt`
 */
constexpr std::string FromDevice(const std::optional<DeviceType> optional) {
    return detail::OptionalToString(optional, [](auto value) {
        switch (value) {
            case DeviceType::kMouse:
                return std::string{detail::kDeviceTypeMouse};
            case DeviceType::kKeyboard:
                return std::string{detail::kDeviceTypeKeyboard};
            case DeviceType::kTouchpad:
                return std::string{detail::kDeviceTypeTouchpad};
            case DeviceType::kTouchscreen:
                return std::string{detail::kDeviceTypeTouchscreen};
            case DeviceType::kUnknown:
                return std::string{detail::kDeviceTypeUnknown};
            default:
                return std::string{};
        }
    });
}

/**
 * \brief Format a string from an optional `ModifierValue`.
 * \param optional optional modifier value
 * \return string representation of the modifier, or empty string if `nullopt`
 */
constexpr std::string FromModifierValue(const std::optional<ModifierValue> optional) {
    return detail::OptionalToString(optional, [](auto value) {
        switch (value) {
            case ModifierValue::kShift:
                return std::string{detail::kModifierValueShift};
            case ModifierValue::kCapsLock:
                return std::string{detail::kModifierValueCapslock};
            case ModifierValue::kControl:
                return std::string{detail::kModifierValueControl};
            case ModifierValue::kAlt:
                return std::string{detail::kModifierValueAlt};
            case ModifierValue::kNumLock:
                return std::string{detail::kModifierValueNumlock};
            case ModifierValue::kMod3:
                return std::string{detail::kModifierValueMoD3};
            case ModifierValue::kSuper:
                return std::string{detail::kModifierValueSuper};
            case ModifierValue::kMod5:
                return std::string{detail::kModifierValueMoD5};
            default:
                return std::string{};
        }
    });
}

/**
 * \brief Format a string from an optional `EntryType`.
 * \param optional optional entry type value
 * \return string representation of the entry type, or empty string if `nullopt`
 */
constexpr std::string FromEntryType(const std::optional<EntryType> optional) {
    return detail::OptionalToString(optional, [](auto value) {
        switch (value) {
            case EntryType::kKey:
                return std::string{detail::kEntryTypeKey};
            case EntryType::kMouseMove:
                return std::string{detail::kEntryTypeMouseMove};
            case EntryType::kMouseClick:
                return std::string{detail::kEntryTypeMouseClick};
            case EntryType::kMouseScroll:
                return std::string{detail::kEntryTypeMouseScroll};
            default:
                return std::string{};
        }
    });
}

/**
 * \brief Convert an integer string representation to an enum.
 * \tparam Enum enum type to convert to
 * \param value string representation of the underlying integer value
 * \return optional enum value, `nullopt` if string is empty or invalid
 */
template <class Enum>
constexpr std::optional<Enum> FromUnderlying(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    return std::optional{static_cast<Enum>(std::stoi(value))};
}

/**
 * \brief Format a string from an optional `Interval` value.
 * \param optional optional interval value
 * \return string representation of the interval in microseconds, or empty string if `nullopt`
 */
constexpr std::string FromInterval(const std::optional<IntervalType> optional) {
    return detail::OptionalToString(optional, [](auto value) { return std::to_string(value.count()); });
}

/**
 * \brief Format a string from an optional double value.
 * \param optional optional double value
 * \return string representation of the double, or empty string if `nullopt`
 */
constexpr std::string FromDouble(const std::optional<double> optional) {
    return detail::OptionalToString(optional, [](auto value) { return std::to_string(value); });
}
} // namespace evget

#endif
