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

void EvgetCore::Event::Entry::toNamedRepresentation() {
    if (_data.size() >= detail::mouseMoveNFields) {
        _data[detail::mouseMoveNFields - 1] = fromDevice(fromUnderlying<Device>(_data[detail::mouseMoveNFields - 1]));
    }
    if (_data.size() >= detail::mouseClickNFields) {
        _data[detail::mouseClickNFields - 1] = fromButtonAction(fromUnderlying<ButtonAction>(_data[detail::mouseClickNFields - 1]));
    }

   std::ranges::for_each(_modifiers.begin(), _modifiers.end(), [](std::string &modifier) {
       modifier = fromModifierValue(fromUnderlying<ModifierValue>(modifier));
   });
}

EvgetCore::Event::EntryWithFields EvgetCore::Event::Entry::getEntryWithFields() {
    std::vector<std::string> fields;
    switch (this->type()) {
        case EntryType::Key:
            fields = {detail::keyFields.begin(), detail::keyFields.end()};
            break;
        case EntryType::MouseClick:
            fields = {detail::mouseClickFields.begin(), detail::mouseClickFields.end()};
            break;
        case EntryType::MouseMove:
            fields = {detail::mouseMoveFields.begin(), detail::mouseMoveFields.end()};
            break;
        case EntryType::MouseScroll:
            fields = {detail::mouseScrollFields.begin(), detail::mouseScrollFields.end()};
            break;
    }

    return {
        .type = type(),
        .fields = fields,
        .data = data(),
        .modifiers = modifiers()
    };
}

EvgetCore::Event::EntryType EvgetCore::Event::Entry::type() const {
    return _type;
}
