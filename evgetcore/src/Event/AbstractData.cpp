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

#include <evgetcore/UnsupportedOperationException.h>
#include "evgetcore/Event/AbstractData.h"
#include <utility>
#include <fmt/format.h>

Event::AbstractData::~AbstractData() = default;

Event::AbstractData::AbstractData(std::string name) : fields{}, name{std::move(name)} {
}

Event::AbstractData::Iterator Event::AbstractData::begin() const noexcept {
    return fields.begin();
}

Event::AbstractData::Iterator Event::AbstractData::end() const noexcept {
    return fields.end();
}

const Event::AbstractField& Event::AbstractData::getByName(std::string name) const {
    for (const auto& field : fields) {
        if (field->getName() == name) {
            return *field;
        }
    }
    throw evget::UnsupportedOperationException(fmt::format("{} not in event data.", name));
}

const Event::AbstractField& Event::AbstractData::getAtPosition(size_t position) const {
    if (position < fields.size()) {
        return *fields.at(position);
    }
    throw evget::UnsupportedOperationException(fmt::format("{} index out of range.", std::to_string(position)));
}

std::string Event::AbstractData::getName() const {
    return name;
}