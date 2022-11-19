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
#include "Relation.h"

namespace EvgetCore::Event {
//    template<typename T, std::size_t NFields, typename... LinkedTo>
//    concept SchemaConcept = requires(T value) {
//        { value.getFields() } -> std::convertible_to<std::array<SchemaField::Field, NFields> &>;
//        { value.getLinkedTo() } -> std::convertible_to<std::tuple<LinkedTo...> &>;
//    };

    /**
     * A Schema defines the shape of a `Data` entry. This tells the storage component how to store the `Data`.
     */
    template<std::size_t NFields, typename... LinkedTo>
    class Schema {
    public:
        constexpr explicit Schema(std::string_view name, std::array<SchemaField::Field, NFields> fields, Relation<LinkedTo>... linkedTo);

        /**
         * Get the fields in this Schema.
         */
        [[nodiscard]] constexpr const std::array<SchemaField::Field, NFields> &getFields() const;

        /**
         * Get the linked to schemas.
         */
        [[nodiscard]] constexpr const std::tuple<std::pair<bool, LinkedTo>...> &getLinkedTo() const;

    private:
        std::string_view name{};
        std::array<SchemaField::Field, NFields> fields{};
        std::tuple<Relation<LinkedTo>...> linkedTo{};
    };

    template<std::size_t NFields, typename... LinkedTo>
    constexpr
    Schema<NFields, LinkedTo...>::Schema(std::string_view name, std::array<SchemaField::Field, NFields> fields,
                                         Relation<LinkedTo>... linkedTo) : name{name}, fields{fields}, linkedTo{linkedTo...} {

    }

    template<std::size_t NFields, typename... LinkedTo>
    constexpr const std::array<SchemaField::Field, NFields> &Schema<NFields, LinkedTo...>::getFields() const {
        return fields;
    }

    template<std::size_t NFields, typename... LinkedTo>
    constexpr const std::tuple<std::pair<bool, LinkedTo>...> &Schema<NFields, LinkedTo...>::getLinkedTo() const {
        return linkedTo;
    }
}


#endif //EVGET_SCHEMA_H
