#ifndef EVGET_UTIL_H
#define EVGET_UTIL_H

#include <concepts>
#include <type_traits>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/// \brief Stringify a token.
#define EVGET_STRINGIFY(x) #x

// NOLINTEND(cppcoreguidelines-macro-usage)

namespace evget {
/**
 * \brief Invocable concept with a checked return type.
 */
template <class F, class R, class... Args>
concept Invocable = std::invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, R>;
} // namespace evget

#endif
