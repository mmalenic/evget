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

#include <date/tz.h>

#include <array>
#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ButtonAction.h"
#include "DataType.h"
#include "Device.h"
#include "Relation.h"
#include "SchemaField.h"
#include "evgetcore/Util.h"

namespace EvgetCore::Event {
/**
 * A Schema defines the shape of a `Data` entry. This tells the storage component how to store the `Data`.
 */
template <std::size_t NFields, typename... To>
class Schema {
public:
    constexpr Schema(std::string_view name, std::array<SchemaField::Field, NFields> fields, Relation<To>... relations);

    /**
     * Get the fields in this Schema.
     */
    [[nodiscard]] constexpr const std::array<SchemaField::Field, NFields>& getFields() const;

    /**
     * Get the linked to schemas.
     */
    [[nodiscard]] constexpr const std::tuple<Relation<To>...>& getRelations() const;

    /**
     * Get the schema name.
     */
    [[nodiscard]] constexpr const std::string_view& getName() const;

private:
    std::string_view name{};
    std::array<SchemaField::Field, NFields> fields{};
    std::tuple<Relation<To>...> relations{};
};

template <std::size_t NFields, typename... To>
constexpr Schema<NFields, To...>::Schema(
    std::string_view name,
    std::array<SchemaField::Field, NFields> fields,
    Relation<To>... relations
)
    : name{name}, fields{fields}, relations{relations...} {}

template <std::size_t NFields, typename... To>
constexpr const std::array<SchemaField::Field, NFields>& Schema<NFields, To...>::getFields() const {
    return fields;
}

template <std::size_t NFields, typename... To>
constexpr const std::tuple<Relation<To>...>& Schema<NFields, To...>::getRelations() const {
    return relations;
}

template <std::size_t NFields, typename... To>
constexpr const std::string_view& Schema<NFields, To...>::getName() const {
    return name;
}
}  // namespace EvgetCore::Event

#endif  // EVGET_SCHEMA_H
