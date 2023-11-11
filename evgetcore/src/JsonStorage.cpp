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
#include "evgetcore/JsonStorage.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <evgetcore/Event/Schema.h>

void EvgetCore::JsonStorage::store(Event::Data event) {
    nlohmann::json output{};

    auto nodes = event.getNodes();
    auto formattedNodes = std::vector<nlohmann::json>{};
    for (const auto& node : nodes) {
        auto processedFields = std::vector<std::vector<nlohmann::json>>{};
        for (const auto& field : node.second) {
            auto out = std::vector<nlohmann::json>{};
            std::ranges::transform(field.begin(), field.end(),
                                   std::back_inserter(out), [](const auto& field) {
                return nlohmann::json{
                    {"name", getName(field.fieldDefinition)},
                    {"type", getType(field.fieldDefinition)},
                    {"data", field.data},
                };
            });

            processedFields.push_back(out);
        }

        formattedNodes.push_back({node.first, processedFields});
    }

    auto edges = event.getEdges();
    auto formattedEdges = std::vector<nlohmann::json>{};
    for (const auto& from : edges) {
        for (const auto& to : from.second) {
            formattedEdges.push_back(nlohmann::json{
                        {"from", from.first},
                        {"to", to.first},
                // todo make this not use vectors
                        {"relation", getRelation(to.second[0])},
                    });
        }
    }

    output["nodes"] = formattedNodes;
    output["edges"] = formattedEdges;

    ostream.get() << output.dump(4);
}

EvgetCore::JsonStorage::JsonStorage(std::ostream& ostream) : ostream{ostream} {
}
