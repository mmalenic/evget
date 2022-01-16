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

#ifndef EVGET_TEST_TESTUTILS_INCLUDE_TESTUTILS_EVENT_EVENT_H
#define EVGET_TEST_TESTUTILS_INCLUDE_TESTUTILS_EVENT_EVENT_H

#include <string>
#include "evget/Event/Data.h"
#include "gtest/gtest.h"

namespace TestUtils::Event {
    void test_create_and_iterate(auto&& create) {
        std::string field_name = "field";
        std::string entry = "entry";
        Data eventData = create(field_name, "name");
        eventData.setAtPosition(0, "entry");

        auto n = 0;
        for (auto i{eventData.begin()}; i != eventData.end(); i++, n++) {
            ASSERT_EQ(field_name, i->getName());
            ASSERT_EQ(entry, i->getEntry());
        }
        ASSERT_EQ(n, 1);
    }

    void test_get_and_set(auto&& get, std::string entry) {
        std::string field_name = "field";
        Data eventData{"name", {field_name}};
        auto field = get(eventData, field_name, 0, entry);
        ASSERT_EQ(field_name, field.getName());
        ASSERT_EQ(entry, field.getEntry());
    }
}

#endif //EVGET_TEST_TESTUTILS_INCLUDE_TESTUTILS_EVENT_EVENT_H
