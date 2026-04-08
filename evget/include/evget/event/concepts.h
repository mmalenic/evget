/**
 * \file concepts.h
 * \brief Concepts for evget event builders.
 */

#ifndef EVGET_EVENT_CONCEPTS_H
#define EVGET_EVENT_CONCEPTS_H

#include <concepts>
#include <optional>
#include <string>

#include "evget/event/modifier_value.h"
#include "evget/event/schema.h"

namespace evget {
/**
 * \brief Check whether the template parameter is a builder with a modifier function.
 */
template <typename T>
concept BuilderHasModifier = requires(T builder, ModifierValue modifier_value) {
    { builder.Modifier(modifier_value) } -> std::convertible_to<T>;
};

/**
 * \brief Checks whether the template is a builder with the `ButtonName` function.
 */
template <typename T>
concept BuilderHasButtonName = requires(T builder, std::string name) {
    { builder.ButtonName(std::move(name)) } -> std::convertible_to<T>;
};

/**
 * \brief Checks whether the template is a builder with the `DeviceName` function.
 */
template <typename T>
concept BuilderHasDeviceName = requires(T builder, std::string name) {
    { builder.DeviceName(std::move(name)) } -> std::convertible_to<T>;
};

/**
 * \brief Checks whether the template is a builder with base fields on all builders.
 */
template <typename T>
concept BuilderHasBaseFields = evget::BuilderHasModifier<T> && evget::BuilderHasDeviceName<T> &&
    requires(T builder,
             TimestampType timestamp,
             std::optional<IntervalType> interval,
             DeviceType device,
             std::string name,
             std::string device_id) {
        { builder.Timestamp(timestamp) } -> std::convertible_to<T>;
        { builder.Interval(interval) } -> std::convertible_to<T>;
        { builder.Device(device) } -> std::convertible_to<T>;
        { builder.DeviceName(std::move(name)) } -> std::convertible_to<T>;
        { builder.DeviceId(std::move(device_id)) } -> std::convertible_to<T>;
        { builder.SystemEvent(std::move(name)) } -> std::convertible_to<T>;
        { builder.EventSource(std::move(name)) } -> std::convertible_to<T>;
    };

/**
 * \brief Check whether the template parameter is a builder with focus window functions.
 */
template <typename T>
concept BuilderHasWindowFunctions =
    requires(T builder, std::string name, double x_pos, double y_pos, double width, double height) {
        { builder.FocusWindowName(name) } -> std::convertible_to<T>;
        { builder.FocusWindowPositionX(x_pos) } -> std::convertible_to<T>;
        { builder.FocusWindowPositionY(y_pos) } -> std::convertible_to<T>;
        { builder.FocusWindowWidth(width) } -> std::convertible_to<T>;
        { builder.FocusWindowHeight(height) } -> std::convertible_to<T>;
    };

/**
 * \brief Check whether the template parameter is a builder with the Screen function.
 */
template <typename T>
concept BuilderHasScreenFunction = requires(T builder, std::string device_name, int screen) {
    { builder.Screen(screen) } -> std::convertible_to<T>;
};

} // namespace evget

#endif // EVGET_EVENT_CONCEPTS_H
