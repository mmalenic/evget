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

#include <string>

#include "evget/Error.h"

namespace evget {

/**
 * \brief An interface representing a query.
 */
class Query {
public:
    /**
     * \brief Bind an integer to the position.
     */
    virtual void BindInt(int position, int value) = 0;

    /**
     * \brief Bind a double to the position.
     */
    virtual void BindDouble(int position, double value) = 0;

    /**
     * \brief Bind a character array to the position.
     */
    virtual void BindChars(int position, const char* value) = 0;

    /**
     * \brief Bind a boolean to the position.
     */
    virtual void BindBool(int position, bool value) = 0;

    /**
     * \brief Reset this query. This should not reset a transaction.
     */
    virtual Result<void> Reset() = 0;

    /**
     * \brief Get the next query.
     * \return result indicating if successful. True means that there are more steps to execute.
     */
    virtual Result<bool> Next() = 0;

    /**
     * \brief Advance the query until there are no more steps to execute.
     * \return result indicating if successful.
     */
    virtual Result<void> NextWhile() = 0;

    /**
     * \brief Execute a query directly against the database connection without preparing it.
     * \return result indicating if successful.
     */
    virtual Result<void> Exec() = 0;

    /**
     * \brief Get the field as an integer.
     * \return a result with the returned integer.
     */
    virtual Result<int> AsInt(int pos) = 0;

    /**
     * \brief Get the field as a double.
     * \return a result with the returned double.
     */
    virtual Result<double> AsDouble(int pos) = 0;

    /**
     * \brief Get the field as a string.
     * \return a result with the returned string.
     */
    virtual Result<std::string> AsString(int pos) = 0;

    /**
     * \brief Get the field as a boolean.
     * \return a result with the returned boolean.
     */
    virtual Result<bool> AsBool(int pos) = 0;

    Query() = default;

    virtual ~Query() = default;

    Query(const Query&) = delete;
    Query(Query&&) noexcept = delete;
    Query& operator=(const Query&) = delete;
    Query& operator=(Query&&) noexcept = delete;
};
}  // namespace evget

#endif  // DATABASE_QUERY_H
