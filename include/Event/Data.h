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

#include "Field.h"

/**
 * Event container to represent event data.
 */
class Data {
public:
    using iterator = std::vector<Field>::iterator;

    /**
     * Construct the event data.
     * @param name name of event data
     * @param fields fields
     */
    Data(std::string name, std::initializer_list<Field> fields);

    /**
     * Construct the event data.
     * @param name name of event data
     * @param fieldsNames field names
     */
    Data(std::string name, std::initializer_list<std::string> fieldsNames);

    /**
     * Iterator begin.
     * @return iterator begin
     */
    [[nodiscard]] iterator begin() noexcept;

    /**
     * Iterator end.
     * @return iterator end
     */
    [[nodiscard]] iterator end() noexcept;

    /**
     * Get the field.
     * @param name by name
     */
    Field getByName(std::string name);

    /**
     * Set the field.
     * @param name with name
     */
    void setForName(std::string name, const std::string& entry);

    /**
     * Get the field.
     * @param position position
     */
    Field getAtPosition(size_t position);

    /**
     * Set the field.
     * @param position with position
     */
    void setAtPosition(size_t position, const std::string& entry);

    /**
     * Get the number of fields.
     * @return number of fields
     */
    [[nodiscard]] size_t numberOfFields() const;

    /**
     * Get the name of the data.
     * @return name
     */
    [[nodiscard]] std::string getName() const;

private:
    const size_t nFields;
    const std::string name;
    std::vector<Field> fields;

    /**
     * Get the field.
     * @param name by name
     */
    Field& getReferenceByName(std::string name);

    /**
     * Get the field.
     * @param position position
     */
    Field& getReferenceAtPosition(size_t position);
};

#endif //EVGET_INCLUDE_EVENTDATA_H
