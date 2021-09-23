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

#include <tuple>
#include <UnsupportedOperationException.h>
#include <event_data/EventData.h>
#include <utility>

using namespace std;

EventData::EventData(string name, initializer_list<Field> fields) : nFields{fields.size()}, name{std::move(name)}, fields{fields} {
}

EventData::EventData(string name, initializer_list<string> fieldNames) : nFields{fieldNames.size()}, name{std::move(name)}, fields{} {
    fields.reserve(nFields);
    for (const auto& fieldName : fieldNames) {
        fields.emplace_back(fieldName);
    }
}

EventData::iterator EventData::begin() noexcept {
    return fields.begin();
}

EventData::iterator EventData::end() noexcept {
    return fields.end();
}

size_t EventData::numberOfFields() const {
    return nFields;
}

Field EventData::getByName(std::string name) {
    return getReferenceByName(std::move(name));
}

Field EventData::getAtPosition(size_t position) {
    return getReferenceAtPosition(position);
}

std::string EventData::getName() const {
    return name;
}

void EventData::setForName(std::string name, const std::string& entry) {
    auto& field = getReferenceByName(name);
    field.setEntry(entry);
}

void EventData::setAtPosition(size_t position, const std::string& entry) {
    auto& field = getReferenceAtPosition(position);
    field.setEntry(entry);
}

Field& EventData::getReferenceByName(std::string name) {
    auto result = find_if(begin(), end(), [&name](const Field& field) { return field.getName() == name; });
    if (result != end()) {
        return *result;
    }
    throw UnsupportedOperationException(name + " not in event data.");
}

Field& EventData::getReferenceAtPosition(size_t position) {
    if (position < fields.size()) {
        return fields.at(position);
    }
    throw UnsupportedOperationException(to_string(position) + " index out of range.");
}