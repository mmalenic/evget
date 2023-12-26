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

#ifndef UTIL_H
#define UTIL_H

#include <expected>
#include <string>

namespace Util {
/**
 * Invocable concept with a checked return type.
 */
template <class F, class R, class... Args>
concept Invocable = std::invocable<F, Args...> && std::convertible_to<std::invoke_result_t<F, Args...>, R>;

/**
 * \brief Error struct.
 */
template<typename E>
struct Error {
    E errorType;
    std::string message;
};

/**
 * \brief Result type.
 */
template<typename T, typename E>
using Result = std::expected<T, Error<E>>;

/**
 * \brief Error type.
 */
template<typename E>
using Err = std::unexpected<Error<E>>;
}

#endif //UTIL_H
