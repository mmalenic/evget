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

#ifndef EVGET_INCLUDE_EVENT_DATA_FIELD_H
#define EVGET_INCLUDE_EVENT_DATA_FIELD_H

#include <string>
#include <variant>
#include <memory>
#include <vector>

namespace EvgetCore::Event {

    class AbstractData;

    /**
     * Represents a field in an event.
     */
    class AbstractField {
    public:
        using Entries = std::vector<std::unique_ptr<AbstractData>>;
        using Iterator = Entries::const_iterator;
        using EntryOrData = std::variant<std::string, Entries>;

        AbstractField(std::string name, Entries entries);
        AbstractField(std::string name, std::string entry);
        explicit AbstractField(std::string name);

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

        [[nodiscard]] Iterator begin() const noexcept;

        [[nodiscard]] Iterator end() const noexcept;

        virtual ~AbstractField() = 0;

        AbstractField(const AbstractField &) = delete;

        AbstractField &operator=(const AbstractField &) = delete;

    protected:
        AbstractField(AbstractField &&) noexcept = default;

        AbstractField &operator=(AbstractField &&) noexcept = default;

    private:
        std::string name;
        EntryOrData entry;
    };

    constexpr bool Event::AbstractField::isEntry() {
        return std::holds_alternative<std::string>(entry);
    }

    constexpr bool Event::AbstractField::isData() {
        return std::holds_alternative<Entries>(entry);
    }
}

#endif //EVGET_INCLUDE_EVENT_DATA_FIELD_H
