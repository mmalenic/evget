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

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "evgetcore/Error.h"
#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Schema.h"

EvgetCore::Result<void> EvgetCore::Storage::JsonStorage::store(Event::Data event) {
    if (event.empty()) {
        return Result<void>{};
    }

    auto formattedEntries = std::vector<nlohmann::json>{};
    for (auto entry : event.entries()) {
        entry.toNamedRepresentation();
        auto entryWithFields = entry.getEntryWithFields();

        auto formattedFields = std::vector<nlohmann::json>{};
        for (auto i = 0; i < entryWithFields.data.size(); i++) {
            formattedFields.push_back({{"name", entryWithFields.fields[i]}, {"data", entryWithFields.data[i]}});
        }
        formattedEntries.push_back(
            {{"type", fromEntryType(entryWithFields.type)},
             {"fields", formattedFields},
             {"modifiers", entryWithFields.modifiers}}
        );
    }

    nlohmann::json output{};
    output["entries"] = formattedEntries;

    std::visit([output](auto&& ostream) { *ostream << output.dump(4) << "\n"; }, ostream);

    return Result<void>{};
}

EvgetCore::Storage::JsonStorage::JsonStorage(std::unique_ptr<std::ostream> ostream) : ostream{std::move(ostream)} {}

EvgetCore::Storage::JsonStorage::JsonStorage(std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> ostream)
    : ostream{std::move(ostream)} {}
