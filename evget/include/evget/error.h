#ifndef EVGET_ERROR_H
#define EVGET_ERROR_H

#include <concepts>
#include <cstdint>
#include <expected>
#include <string>
#include <type_traits>

namespace evget {
/**
 * Invocable concept with a checked return type.
 */
template <class F, class R, class... Args>
concept Invocable = std::invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, R>;

/**
 * \brief Error struct.
 */
template <typename E>
struct Error {
    E error_type;
    std::string message;
};

/**
 * \brief Error type enum.
 */
enum class ErrorType : std::uint8_t {
    kSqLiteError,
    kDatabaseManager,
    kDatabaseError,
    kEventHandlerError,
    kAsyncError,
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
}  // namespace evget

#endif
