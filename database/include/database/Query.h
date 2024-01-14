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

#ifndef DATABASE_QUERY_H
#define DATABASE_QUERY_H

#include "Error.h"

namespace Database {

/**
 * \brief An interface representing a query.
 */
class Query {
public:
    /**
     * \brief Bind an integer to the position.
     */
    virtual void bindInt(std::size_t position, int value) = 0;

    /**
     * \brief Bind a double to the position.
     */
    virtual void bindDouble(std::size_t position, double value) = 0;

    /**
     * \brief Bind a character array to the position.
     */
    virtual void bindChars(std::size_t position, const char* value) = 0;

    /**
     * \brief Bind a boolean to the position.
     */
    virtual void bindBool(std::size_t position, bool value) = 0;

    /**
     * \brief Reset this query. This should not reset a transaction.
     */
    virtual Result<void> reset() = 0;

    /**
     * \brief Get the next query.
     * \return result indicating if successful. True means that there are more steps to execute.
     */
    virtual Result<bool> next() = 0;

    /**
     * \brief Get the field as an integer.
     * \return a result with the returned integer.
     */
    virtual Result<int> asInt(std::size_t at) = 0;

    /**
     * \brief Get the field as a double.
     * \return a result with the returned double.
     */
    virtual Result<double> asDouble(std::size_t at) = 0;

    /**
     * \brief Get the field as a string.
     * \return a result with the returned string.
     */
    virtual Result<std::string> asString(std::size_t at) = 0;

    /**
     * \brief Get the field as a boolean.
     * \return a result with the returned boolean.
     */
    virtual Result<bool> asBool(std::size_t at) = 0;

    Query() = default;

    virtual ~Query() = default;

    Query(const Query&) = delete;
    Query(Query&&) noexcept = delete;
    Query& operator=(const Query&) = delete;
    Query& operator=(Query&&) noexcept = delete;
};
}


#endif // DATABASE_QUERY_H
