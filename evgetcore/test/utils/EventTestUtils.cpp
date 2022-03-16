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

#include "EventTestUtils.h"

void TestUtils::EventTestUtils::fieldValueAndName(const Event::AbstractField& field, const std::string& name, const std::string& expected) {
    ASSERT_EQ(name, field.getName());
    ASSERT_EQ(expected, field.getEntry());
}

std::vector<std::unique_ptr<Event::AbstractField>> TestUtils::EventTestUtils::allocateFields(const std::string& name, const std::string& entry) {
    std::vector<std::unique_ptr<Event::AbstractField>> fields{};
    fields.emplace_back(std::make_unique<Event::Field>(name, entry));
    return fields;
}

Event::Field TestUtils::EventTestUtils::constructRecursiveField(const std::string& outerName, const std::string& innerDataName, const std::string& innerName, const std::string& innerEntry) {
    Event::AbstractField::Entries entries{};
    std::vector<std::unique_ptr<Event::AbstractField>> fields = TestUtils::EventTestUtils::allocateFields(innerName, innerEntry);
    auto data = std::make_unique<Event::Data>(innerDataName, std::move(fields));
    entries.emplace_back(std::move(data));
    return Event::Field{outerName, std::move(entries)};
}