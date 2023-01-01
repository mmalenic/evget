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

#ifndef EVGET_DATAWITHSCHEMA_H
#define EVGET_DATAWITHSCHEMA_H

#include "Data.h"

namespace EvgetCore::Event {

/**
 * Event container to represent event data with an associated Schema.
 */
template<typename Schema>
class DataWithSchema {
public:
    DataWithSchema() = default;
    /**
     * Create a DataWithSchema.
     */
    DataWithSchema(const Data& data, const Schema& schema);

private:
    std::reference_wrapper<const Data> data;
    std::reference_wrapper<const Schema> schema;
};

template <typename Schema>
DataWithSchema<Schema>::DataWithSchema(const Data& data, const Schema& schema) : data{data}, schema{schema} {
}

}

template <typename Schema>
struct fmt::formatter<EvgetCore::Event::DataWithSchema<Schema>> : fmt::formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const EvgetCore::Event::Data& data, FormatContext& context) const -> decltype(context.out()) {
        auto out = fmt::format("[{}]\n{}\n\n", data.getName(), fmt::join(data, " "));

        for (const auto& [_, containedData] : data.getData()) {
            out += fmt::format("[{}] -> {}", data.getName(), fmt::join(containedData, " "));
        }

        return formatter<string_view>::format(out, context);
    }
};

#endif  // EVGET_DATAWITHSCHEMA_H
