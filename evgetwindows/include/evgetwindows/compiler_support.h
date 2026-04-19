/**
 * \file compiler_support.h
 * \brief Compile-time assertions for the Windows backend.
 */

#ifndef EVGETWINDOWS_COMPILER_SUPPORT_H
#define EVGETWINDOWS_COMPILER_SUPPORT_H

#include <version>

static_assert(
    __cpp_lib_expected >= 202202L,
    "evget requires C++23 std::expected (__cpp_lib_expected >= 202202L). "
    "Upgrade to MSVC 17.11+, clang-cl 19+, or MinGW-w64 GCC 14.2+."
);

static_assert(
    __cpp_lib_format >= 202110L,
    "evget requires C++23 std::format enhancements (__cpp_lib_format >= 202110L). "
    "MinGW-w64 GCC 14.0 has a regression, use 14.2+."
);

static_assert(
    __cpp_lib_ranges_to_container >= 202202L,
    "evget requires C++23 std::ranges::to (__cpp_lib_ranges_to_container >= 202202L). "
    "Upgrade compiler."
);

static_assert(__cpp_concepts >= 201907L, "C++20 concepts support missing.");

static_assert(__cpp_impl_coroutine >= 201902L, "C++20 coroutine support missing.");

#endif // EVGETWINDOWS_COMPILER_SUPPORT_H
