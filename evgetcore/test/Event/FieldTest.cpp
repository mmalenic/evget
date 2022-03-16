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

#include <gtest/gtest.h>
#include "evgetcore/Event/AbstractField.h"
#include "EventTestUtils.h"

namespace EventTestUtils = TestUtils::EventTestUtils;

TEST(FieldTest, ConstructorNoEntry) { // NOLINT(cert-err58-cpp)
    EventTestUtils::fieldValueAndName(Event::Field{"name"}, "name", "");
}

TEST(FieldTest, ConstructorEntry) { // NOLINT(cert-err58-cpp)
    EventTestUtils::fieldValueAndName(Event::Field{"name", "entry"}, "name", "entry");
}

TEST(FieldTest, ConstructorData) { // NOLINT(cert-err58-cpp)
    auto field = EventTestUtils::constructRecursiveField("OuterField", "InnerData", "InnerField", "InnerEntry");

    ASSERT_EQ("InnerField", (*field.begin())->getAtPosition(0).getName());
    ASSERT_EQ("InnerEntry", (*field.begin())->getAtPosition(0).getEntry());
    ASSERT_EQ("InnerData", (*field.begin())->getName());
    ASSERT_EQ("OuterField", field.getName());
}

TEST(FieldTest, GetName) { // NOLINT(cert-err58-cpp)
    Event::Field field{"name", "entry"};
    ASSERT_EQ("name", field.getName());
}

TEST(FieldTest, GetEntry) { // NOLINT(cert-err58-cpp)
    Event::Field field{"name", "entry"};
    ASSERT_EQ("entry", field.getEntry());
}

TEST(FieldTest, IsEntry) { // NOLINT(cert-err58-cpp)
    Event::Field field{"name", "entry"};
    ASSERT_TRUE(field.isEntry());
    ASSERT_FALSE(field.isData());
}

TEST(FieldTest, IsData) { // NOLINT(cert-err58-cpp)
    Event::Field field = EventTestUtils::constructRecursiveField("OuterField", "InnerData", "InnerField", "InnerEntry");
    ASSERT_TRUE(field.isData());
    ASSERT_FALSE(field.isEntry());
}