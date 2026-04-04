/**
 * \file error.h
 * \brief Error handling types and utilities for the evget library.
 */

#ifndef EVGET_ERROR_H
#define EVGET_ERROR_H

#include <spdlog/spdlog.h>

#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <type_traits>

#include "event/modifier_value.h"
#include "event/schema.h"

namespace evget {
/**
 * \brief Invocable concept with a checked return type.
 */
template <class F, class R, class... Args>
concept Invocable = std::invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, R>;

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

/**
 * \brief Error struct.
 */
template <typename E>
struct Error {
    E error_type; ///< The specific error type
    std::string message; ///< Error message
};

/**
 * \brief Error type enum.
 */
enum class ErrorType : std::uint8_t {
    kSqLiteError, ///< SQLite database error
    kDatabaseManagerError, ///< Database manager error
    kDatabaseError, ///< General database error
    kEventHandlerError, ///< Event handler error
    kAsyncError, ///< Asynchronous operation error
};

/**
 * \brief Result type.
 */
template <typename T>
using Result = std::expected<T, Error<ErrorType>>;

/**
 * \brief Error type.
 */
using Err = std::unexpected<Error<ErrorType>>;

} // namespace evget

/**
 * \brief Defines the
 * [`std:formatter`](https://en.cppreference.com/w/cpp/utility/format/formatter)
 * for formatting `evget::Error<ErrorType>`.
 */
template <>
struct std::formatter<evget::Error<evget::ErrorType>> {
    /**
     * \brief Parse the input devices by beginning a new iterator from the context.
     *
     * \param ctx formatting context
     * \return output iterator
     */
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr auto parse(const std::format_parse_context& ctx) {
        return ctx.begin();
    }

    /**
     * \brief Format the errror based on the output `Format`.
     *
     * \tparam Context context type
     * \param error error result
     * \param ctx context parameter
     * \return iterator after formatting
     */
    template <typename Context>
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr auto format(const evget::Error<evget::ErrorType>& error, Context& ctx) const {
        std::string error_type;
        switch (error.error_type) {
            case evget::ErrorType::kSqLiteError:
                error_type = "SqLiteError";
                break;
            case evget::ErrorType::kDatabaseManagerError:
                error_type = "DatabaseManagerError";
                break;
            case evget::ErrorType::kDatabaseError:
                error_type = "DatabaseError";
                break;
            case evget::ErrorType::kEventHandlerError:
                error_type = "EventHandlerError";
                break;
            case evget::ErrorType::kAsyncError:
                error_type = "AsyncError";
                break;
        }

        return std::format_to(ctx.out(), "{}: {}", error_type, error.message);
    }
};

#endif
