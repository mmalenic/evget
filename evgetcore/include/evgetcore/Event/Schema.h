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
//

#ifndef EVGET_SCHEMA_H
#define EVGET_SCHEMA_H

#include <map>
#include <string>
#include <optional>
#include <chrono>
#include <array>
#include <utility>
#include <date/tz.h>
#include "ButtonAction.h"
#include "Device.h"
#include "evgetcore/Util.h"
#include "DataType.h"
#include "SchemaField.h"

namespace EvgetCore::Event {
    /**
     * A Schema defines the shape of a `Data` entry. This tells the storage component how to store the `Data`.
     */
    template<std::size_t NFields = 0, std::size_t NLinkedTo = 0, std::size_t NUniquelyLinkedTo = 0>
    class Schema {
    public:
        constexpr explicit Schema(std::string_view name, std::array<SchemaField::Field, NFields> fields = {}, std::array<Schema, NLinkedTo> linkedTo = {}, std::array<Schema, NUniquelyLinkedTo> uniquelyLinkedTo = {});

        /**
         * Get the fields in this Schema.
         */
        [[nodiscard]] constexpr const std::array<SchemaField::Field, NFields> &getFields() const;

        /**
         * Get the linked to schemas.
         */
        [[nodiscard]] constexpr const std::array<Schema, NLinkedTo> &getLinkedTo() const;

        /**
         * Get the uniquely linked to schemas.
         */
        [[nodiscard]] constexpr const std::array<Schema, NUniquelyLinkedTo> &getUniquelyLinkedTo() const;

    private:
        std::string_view name{};
        std::array<SchemaField::Field, NFields> fields{};
        std::array<Schema, NLinkedTo> linkedTo{};
        std::array<Schema, NUniquelyLinkedTo> uniquelyLinkedTo{};
    };

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::Schema(std::string_view name, std::array<SchemaField::Field, NFields> fields,
                                                                    std::array<Schema, NLinkedTo> linkedTo,
                                                                    std::array<Schema, NUniquelyLinkedTo> uniquelyLinkedTo):
                                                                    name{name},
                                                                    fields{std::move(fields)},
                                                                    linkedTo{std::move(linkedTo)},
                                                                    uniquelyLinkedTo{std::move(uniquelyLinkedTo)} {
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr const std::array<SchemaField::Field, NFields> &Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::getFields() const {
        return fields;
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr const std::array<Schema<NFields, NLinkedTo, NUniquelyLinkedTo>, NLinkedTo> &Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::getLinkedTo() const {
        return linkedTo;
    }

    template<std::size_t NFields, std::size_t NLinkedTo, std::size_t NUniquelyLinkedTo>
    constexpr const std::array<Schema<NFields, NLinkedTo, NUniquelyLinkedTo>, NUniquelyLinkedTo> &Schema<NFields, NLinkedTo, NUniquelyLinkedTo>::getUniquelyLinkedTo() const {
        return uniquelyLinkedTo;
    }
}


#endif //EVGET_SCHEMA_H
