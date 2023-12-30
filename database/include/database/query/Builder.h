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

#ifndef BUILDER_H
#define BUILDER_H

#include "RowIterator.h"
#include "database/Connection.h"
#include "database/Error.h"

namespace Database::Query {

/**
 * \brief An interface representing a query builder.
 */
class Builder {
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
     * \brief Reset this query.
     */
    virtual void reset() = 0;

    /**
     * \brief Build the query, returning a row that needs to be iterated over.
     * \return the row iterator.
     */
    virtual Result<std::reference_wrapper<RowIterator>> build() = 0;

    Builder() = default;

    virtual ~Builder() = default;

    Builder(const Builder&) = delete;
    Builder(Builder&&) noexcept = delete;
    Builder& operator=(const Builder&) = delete;
    Builder& operator=(Builder&&) noexcept = delete;
};
}


#endif //BUILDER_H
