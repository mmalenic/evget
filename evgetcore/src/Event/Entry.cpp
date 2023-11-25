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

#include "evgetcore/Event/Entry.h"

#include "evgetcore/Event/Schema.h"

EvgetCore::Event::Entry::Entry(EntryType type, std::vector<std::string> data, std::vector<std::string> modifiers)
    : _type{type}, _data{std::move(data)}, _modifiers{std::move(modifiers)} {
}

const std::vector<std::string>& EvgetCore::Event::Entry::data() const {
    return _data;
}

const std::vector<std::string>& EvgetCore::Event::Entry::modifiers() const {
    return _modifiers;
}

EvgetCore::Event::Entry EvgetCore::Event::Entry::getNamedRepresentation() {
    auto data{_data};

    if (data.size() >= detail::mouseMoveNFields) {
        data[detail::mouseMoveNFields - 1] = fromUnderlying<Device>(data[detail::mouseMoveNFields - 1]);
    }
    if (data.size() >= detail::mouseClickNFields) {
        data[detail::mouseClickNFields - 1] = fromUnderlying<ButtonAction>(data[detail::mouseClickNFields - 1]);
    }

    std::vector<std::string> modifiers{};
    modifiers.reserve(_modifiers.size());
    std::transform(_data.begin(), _data.end(), std::back_inserter(modifiers), [](auto value) {
        return fromUnderlying<ModifierValue>(value);
    });

    return {
        this->type(),
        data,
        modifiers
    };
}

std::string EvgetCore::Event::Entry::keyFieldName(size_t position) {
    return detail::keyFields.at(position);
}

std::string EvgetCore::Event::Entry::mouseMoveFieldName(size_t position) {
    return detail::mouseMoveFields.at(position);
}

std::string EvgetCore::Event::Entry::mouseScrollFieldName(size_t position) {
    return detail::mouseScrollFields.at(position);
}

std::string EvgetCore::Event::Entry::mouseClickFieldName(size_t position) {
    return detail::mouseClickFields.at(position);
}

EvgetCore::Event::EntryType EvgetCore::Event::Entry::type() const {
    return _type;
}
