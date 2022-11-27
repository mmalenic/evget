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

#include "evgetcore/Event/Data.h"
#include <utility>

EvgetCore::Event::Data::Data(std::string name) : name{std::move(name)}, fields{} {
}

EvgetCore::Event::Data::Iterator EvgetCore::Event::Data::begin() const noexcept {
    return fields.begin();
}

EvgetCore::Event::Data::Iterator EvgetCore::Event::Data::end() const noexcept {
    return fields.end();
}

std::string EvgetCore::Event::Data::getName() const {
    return name;
}

void EvgetCore::Event::Data::addField(std::string field) {
    fields.emplace_back(std::move(field));
}

const std::string& EvgetCore::Event::Data::getFieldAt(size_t position) const {
    return fields.at(position);
}

void EvgetCore::Event::Data::contains(EvgetCore::Event::Data data) {
    containsData.try_emplace(data.getName(), std::vector<Data>{}).first->second.emplace_back(std::move(data));
}

const EvgetCore::Event::Data::ContainedData &EvgetCore::Event::Data::getData() const {
    return containsData;
}

void EvgetCore::Event::Data::contains(std::string name, std::vector<Data> data) {
    auto& value = containsData.try_emplace(name, std::vector<Data>{}).first->second;
    value.insert(value.end(), data.begin(), data.end());
}
