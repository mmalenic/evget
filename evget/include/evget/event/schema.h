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
enum class DataType : std::uint8_t { kString, kInteger, kTimestamp, kInterval, kDouble };

/**
 * A relation between edges in a graph.
 */
enum class Relation : std::uint8_t { kOneToOne, kOneToMany, kManyToOne, kManyToMany };

using IntervalType = std::chrono::microseconds;
using TimestampType = std::chrono::time_point<std::chrono::system_clock>;

using FieldDefinition = std::pair<std::string_view, DataType>;

/**
 * A field represents the name, data type and data of a field.
 */
struct Field {
    FieldDefinition field_definition;
    std::string data;
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
 * Get the name of the FieldDefinition.
 */
constexpr std::string_view GetName(FieldDefinition field) {
    return field.first;
}

/**
 * Get the type of the FieldDefinition.
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
 * Get the type of the FieldDefinition.
 */
constexpr std::string_view GetRelation(Relation relation) {
    switch (relation) {
        case Relation::kOneToOne:
            return detail::kRelationOneToOne;
        case Relation::kOneToMany:
            return detail::kRelationOneToMany;
        case Relation::kManyToOne:
            return detail::kRelationManyToOne;
        case Relation::kManyToMany:
            return detail::kRelationManyToMany;
    }

    return "";
}

/**
 * Convert an enum to its underlying value as a string.
 */
template <class Enum>
constexpr std::string ToUnderlyingOptional(std::optional<Enum> value) {
    return detail::OptionalToString(value, [](auto value) { return ToUnderlying(value); });
}

/**
 * Convert an enum to its underlying value as a string.
 */
template <class Enum>
constexpr std::string ToUnderlying(Enum value) {
    return std::to_string(std::to_underlying(value));
}

/**
 * Create a string from a string value.
 */
constexpr std::string FromString(std::optional<std::string> value) {
    return detail::OptionalToString(std::move(value), [](auto value) { return value; });
}

/**
 * Format a string from an int value.
 */
constexpr std::string FromInt(std::optional<int> value) {
    return detail::OptionalToString(value, [](auto value) { return std::to_string(value); });
}

/**
 * Format a string from a `Timestamp` value.
 */
constexpr std::string FromTimestamp(const std::optional<TimestampType> optional) {
    return detail::OptionalToString(optional, [](auto value) {
        std::stringstream stream{};
        stream << std::format("{:%Y-%m-%dT%H:%M:%S%z}", value);
        return stream.str();
    });
}

/**
 * Format a string from a `ButtonAction` value.
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
 * Format a string from a `Device` value.
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
 * Format a string from a `ModifierValue`.
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
 * Format a string from a `EntryType`.
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
 * Convert an integer string representation to an enum.
 */
template <class Enum>
constexpr std::optional<Enum> FromUnderlying(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    return std::optional{static_cast<Enum>(std::stoi(value))};
}

/**
 * Format a string from an `Interval` value.
 */
constexpr std::string FromInterval(const std::optional<IntervalType> optional) {
    return detail::OptionalToString(optional, [](auto value) { return std::to_string(value.count()); });
}

/**
 * Format a string from a double value.
 */
constexpr std::string FromDouble(const std::optional<double> optional) {
    return detail::OptionalToString(optional, [](auto value) { return std::to_string(value); });
}
} // namespace evget

#endif
