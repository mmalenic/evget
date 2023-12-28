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

#ifndef ROWITERATOR_H
#define ROWITERATOR_H

#include <optional>

#include "Row.h"

namespace Database::Query {

/**
 * \brief An interface representing an iterator over database rows.
 */
class RowIterator {
public:
    /**
     * \brief Get the next row. Note that an iterator may need to be progressed to actually
     * perform any database queries.
     * \return an optional value of rows, where a null value indicates that there are no more rows.
     */
    virtual Result<std::optional<std::reference_wrapper<Row>>> next() = 0;

    RowIterator() = default;

    virtual ~RowIterator() = default;

    RowIterator(const RowIterator&) = delete;
    RowIterator(RowIterator&&) noexcept = delete;
    RowIterator& operator=(const RowIterator&) = delete;
    RowIterator& operator=(RowIterator&&) noexcept = delete;
};
}

#endif //ROWITERATOR_H
