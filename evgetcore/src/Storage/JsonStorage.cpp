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
#include "evgetcore/Storage/JsonStorage.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <evgetcore/Event/Schema.h>

EvgetCore::Storage::Result<void> EvgetCore::Storage::JsonStorage::store(Event::Data event) {
    if (event.empty()) {
        return {};
    }

    auto formattedEntries = std::vector<nlohmann::json>{};
    for (auto entry : event.entries()) {
        entry.toNamedRepresentation();
        auto entryWithFields = entry.getEntryWithFields();

        auto formattedFields = std::vector<nlohmann::json>{};
        for (auto i = 0; i < entryWithFields.data.size(); i++) {
            formattedFields.push_back({
                {"type", entryWithFields.type},
                {"name", entryWithFields.fields[i]},
                {"data", entryWithFields.data[i]},
                {"modifiers", entryWithFields.modifiers[i]}
            });
        }
        formattedEntries.emplace_back(formattedFields);
    }

    nlohmann::json output{};
    output["entries"] = formattedEntries;

    ostream.get() << output.dump(4);

    return {};
}

EvgetCore::Storage::JsonStorage::JsonStorage(std::ostream& ostream) : ostream{ostream} {
}
