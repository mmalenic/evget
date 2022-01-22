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

#ifndef EVGET_INCLUDE_EVENTDATA_H
#define EVGET_INCLUDE_EVENTDATA_H

#include <any>
#include <vector>
#include <string>
#include <variant>
#include <optional>

#include "Field.h"

/**
 * Event container to represent event data.
 */
class Data {
public:
    using iterator = std::vector<Field>::iterator;

    /**
     * List of fields represent the ordered fields in the data.
     */
    Data(std::string name, std::initializer_list<Field> fields);

    /**
     * Get the field.
     */
    Field getByName(std::string name);

    /**
     * Get the field.
     */
    Field getAtPosition(size_t position);

    /**
     * Get the number of fields.
     */
    [[nodiscard]] size_t numberOfFields() const;

    /**
     * Get the name of the data.
     */
    [[nodiscard]] std::string getName() const;

    [[nodiscard]] iterator begin() noexcept;
    [[nodiscard]] iterator end() noexcept;

    virtual ~Data() = default;

    Data(Data&&) noexcept = default;
    Data& operator=(Data&&) noexcept = default;

    Data(const Data&) = default;
    Data& operator=(const Data&) = default;

private:
    size_t nFields;
    std::string name;
    std::vector<Field> fields;
};

#endif //EVGET_INCLUDE_EVENTDATA_H
