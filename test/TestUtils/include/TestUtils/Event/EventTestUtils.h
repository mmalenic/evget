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

#ifndef EVGET_TEST_TESTUTILS_INCLUDE_TESTUTILS_EVENT_EVENTTESTUTILS_H
#define EVGET_TEST_TESTUTILS_INCLUDE_TESTUTILS_EVENT_EVENTTESTUTILS_H

#include <string>
#include "gtest/gtest.h"
#include "evget/Event/Data.h"
#include "evget/Event/Field.h"

namespace TestUtils::EventTestUtils {
    void createAndIterate(auto&& create) {
        std::string field_name = "field";
        std::string entry = "entry";
        Event::Data eventData = create(field_name, "name", "entry");

        auto n = 0;
        for (auto i{eventData.begin()}; i != eventData.end(); i++, n++) {
            ASSERT_EQ(field_name, (*i)->getName());
            ASSERT_EQ(entry, (*i)->getEntry());
        }
        ASSERT_EQ(n, 1);
    }

    std::vector<std::unique_ptr<Event::AbstractField>> allocateFields(const std::string& name, const std::string& entry);

    void getAndSet(auto&& get, std::string entry) {
        std::string field_name = "field";
        Event::Data eventData{"name", allocateFields(field_name, entry)};
        const auto& field = get(eventData, field_name, 0);

        ASSERT_EQ(field_name, field.getName());
        ASSERT_EQ(entry, field.getEntry());
    }

    void fieldValueAndName(const Event::AbstractField& field, const std::string& name, const std::string& expected);

    template<typename T>
    void fieldValueAndName(const char* value, const std::string& name, const std::string& expected) {
        std::unique_ptr<Event::AbstractField> field = std::make_unique<T>(value);
        fieldValueAndName(*field, name, expected);

        std::unique_ptr<Event::AbstractField> fieldDefault = std::make_unique<T>();
        fieldValueAndName(*fieldDefault, name, "");
    }

    template<typename T>
    void fieldValueAndName(const auto& value, const std::string& name, const std::string& expected) {
        std::unique_ptr<Event::AbstractField> field = std::make_unique<T>(value);
        fieldValueAndName(*field, name, expected);

        fieldValueAndName<T>("value", name, "value");
    }

    void event_entry_at(auto&& event, size_t position, const std::string& expected) {
        ASSERT_EQ(event.getAtPosition(position).getEntry(), expected);
    }
}

#endif //EVGET_TEST_TESTUTILS_INCLUDE_TESTUTILS_EVENT_EVENTTESTUTILS_H
