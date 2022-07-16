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

#ifndef EVGET_SRC_EVENT_FIELD_H
#define EVGET_SRC_EVENT_FIELD_H

#include "AbstractField.h"

namespace EvgetCore::Event {
    /**
     * Represents a field in an event.
     */
    class Field {
    public:
        using Entries = std::vector<std::unique_ptr<AbstractData>>;
        using Iterator = Entries::const_iterator;
        using EntryOrData = std::variant<std::string, Entries>;

        Field(std::string name, Entries entries);
        Field(std::string name, std::string entry);
        explicit Field(std::string name);

        /**
         * Check if variant is the entry.
         */
        constexpr bool isEntry();

        /**
         * Check if variant is more data.
         */
        constexpr bool isData();

        /**
         * Get the entry.
         */
        [[nodiscard]] std::string getEntry() const;

        /**
         * Get the name.
         */
        [[nodiscard]] std::string getName() const;

        [[nodiscard]] Iterator begin() const;

        [[nodiscard]] Iterator end() const;

    private:
        std::string name;
        EntryOrData entry;
    };

    constexpr bool EvgetCore::Event::Field::isEntry() {
        return std::holds_alternative<std::string>(entry);
    }

    constexpr bool EvgetCore::Event::Field::isData() {
        return std::holds_alternative<Entries>(entry);
    }
}

#endif //EVGET_SRC_EVENT_FIELD_H
