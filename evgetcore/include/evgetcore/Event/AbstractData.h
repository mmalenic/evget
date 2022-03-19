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
#include <memory>
#include "AbstractField.h"

namespace EvgetCore::Event {

    /**
     * Event container to represent event data.
     */
    class AbstractData {
    public:
        using Iterator = std::vector<std::unique_ptr<AbstractField>>::const_iterator;

        /**
         * List of fields represent the ordered fields in the data.
         */
        explicit AbstractData(std::string name);

        /**
         * Get the field.
         */
        [[nodiscard]] const AbstractField &getByName(std::string name) const;

        /**
         * Get the field.
         */
        [[nodiscard]] const AbstractField &getAtPosition(size_t position) const;

        /**
         * Get the name of the data.
         */
        [[nodiscard]] std::string getName() const;

        [[nodiscard]] Iterator begin() const noexcept;

        [[nodiscard]] Iterator end() const noexcept;

        virtual ~AbstractData() = 0;

    protected:
        std::vector<std::unique_ptr<AbstractField>> fields;

        AbstractData(AbstractData &&) noexcept = default;

        AbstractData &operator=(AbstractData &&) noexcept = default;

        AbstractData(const AbstractData &) = default;

        AbstractData &operator=(const AbstractData &) = default;

    private:
        std::string name;
    };
}

#endif //EVGET_INCLUDE_EVENTDATA_H
