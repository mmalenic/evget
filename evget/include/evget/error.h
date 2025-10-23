#ifndef EVGET_ERROR_H
#define EVGET_ERROR_H

#include <spdlog/spdlog.h>

#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <type_traits>

namespace evget {
/**
 * \brief Invocable concept with a checked return type.
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
    kDatabaseManagerError,
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
