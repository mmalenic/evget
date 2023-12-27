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

#ifndef QUERYFIELD_H
#define QUERYFIELD_H

#include "database/Error.h"

namespace Database::Query {
/**
 * \brief An interface representing a database query result field.
 */
class Field {
    /**
     * \brief Get the field as an integer.
     * \return a result with the returned integer.
     */
    virtual Result<int> asInt() = 0;

    /**
     * \brief Get the field as a double.
     * \return a result with the returned double.
     */
    virtual Result<double> asDouble() = 0;

    /**
     * \brief Get the field as a string.
     * \return a result with the returned string.
     */
    virtual Result<std::string> asString() = 0;

    /**
     * \brief Get the field as a boolean.
     * \return a result with the returned boolean.
     */
    virtual Result<bool> asBool() = 0;

    Field() = default;

    virtual ~Field() = default;

    Field(const Field&) = delete;
    Field(Field&&) noexcept = delete;
    Field& operator=(const Field&) = delete;
    Field& operator=(Field&&) noexcept = delete;
};
}

#endif //QUERYFIELD_H
