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

#ifndef EVGET_INCLUDE_STORAGE_H
#define EVGET_INCLUDE_STORAGE_H

#include <boost/asio.hpp>

#include "Event/Data.h"
#include "EventListener.h"
#include "Storage.h"

namespace EvgetCore {
namespace asio = boost::asio;

/**
 * A storage class which prints events to stdout.
 */
class PrintEvents : public Storage {
public:
    void store(Event::Data event) override;

    template <std::size_t NFields, typename... To>
    void defineSchemas(Event::Schema<NFields, To...> schema);

private:
    std::unordered_map<std::string_view, std::string_view> schemaNameToFields{};
};

template <std::size_t NFields, typename... To>
void EvgetCore::PrintEvents::defineSchemas(EvgetCore::Event::Schema<NFields, To...> schema) {
    if (!schemaNameToFields.contains(schema.getName())) {
        std::array<std::string_view, NFields> names{};
        auto fields = schema.getFields();

        std::transform(fields.start(), fields.end(), std::back_inserter(names), [](const auto& field) {
            return Event::SchemaField::getName(field);
        });

        schemaNameToFields.emplace(schema.getName(), fmt::format("{}", fmt::join(names, " ")));
    }

    std::apply([this](auto&&... relation) { ((defineSchemas(relation)), ...); }, schema.getRelations());
}

}  // namespace EvgetCore

#endif  // EVGET_INCLUDE_STORAGE_H
