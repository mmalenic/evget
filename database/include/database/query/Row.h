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

#ifndef QUERY_H
#define QUERY_H

#include "Field.h"
#include "database/Error.h"

namespace Database::Query {

/**
 * \brief An interface representing a database query row.
 */
class Row {
public:
    /**
     * \brief Get the field at the position.
     * \return a result with the returned field.
     */
    virtual Result<std::reference_wrapper<Field>> getField(std::size_t at) = 0;

    Row() = default;

    virtual ~Row() = default;

    Row(const Row&) = delete;
    Row(Row&&) noexcept = delete;
    Row& operator=(const Row&) = delete;
    Row& operator=(Row&&) noexcept = delete;
};
}

#endif //QUERY_H
