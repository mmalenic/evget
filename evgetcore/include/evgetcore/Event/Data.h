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

#ifndef EVGET_INCLUDE_EVGET_DATA_H
#define EVGET_INCLUDE_EVGET_DATA_H

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "Graph.h"

namespace EvgetCore::Event {

/**
 * Event container to represent event data.
 */
class Data {
public:
    using Iterator = std::vector<std::string>::const_iterator;

    Data() = default;
    /**
     * Create a Data with the given name.
     */
    explicit Data(std::string_view name);

    /**
     * Add a field to this data.
     */
    void addField(std::string field);

    /**
     * Get the field at the position.
     */
    [[nodiscard]] const std::string& getFieldAt(size_t position) const;

    /**
     * Get the name of the data.
     */
    [[nodiscard]] std::string_view getName() const;

    [[nodiscard]] Iterator begin() const noexcept;

    [[nodiscard]] Iterator end() const noexcept;

private:
    std::string_view name{};
    std::vector<std::string> fields{};
};

using GraphData = Graph<Data>;

}  // namespace EvgetCore::Event

#endif  // EVGET_INCLUDE_EVGET_DATA_H
