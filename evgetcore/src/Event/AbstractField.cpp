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

#include "evgetcore/Event/AbstractField.h"
#include "evgetcore/Event/AbstractData.h"

#include <utility>
#include <memory>

Event::AbstractField::~AbstractField() = default;

std::string Event::AbstractField::getEntry() const {
    return std::get<std::string>(entry);
}

Event::AbstractField::AbstractField(std::string name, std::string entry) : name{std::move(name)}, entry{std::move(entry)} {
}

Event::AbstractField::AbstractField(std::string name) : AbstractField{std::move(name), ""} {
}

Event::AbstractField::AbstractField(std::string name, Entries entries) : name{std::move(name)}, entry{std::move(entries)} {
}

std::string Event::AbstractField::getName() const {
    return name;
}

Event::AbstractField::Iterator Event::AbstractField::begin() const noexcept {
    return std::get<Entries>(entry).begin();
}

Event::AbstractField::Iterator Event::AbstractField::end() const noexcept {
    return std::get<Entries>(entry).end();
}