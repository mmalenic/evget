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
#include "evget/Event/Data.h"
#include "TestUtils/Event/EventTestUtils.h"

#include <utility>
#include "evget/UnsupportedOperationException.h"

namespace EventUtils = TestUtils::EventTestUtils;

TEST(EventDataTest, CreationAndIterationByName) { // NOLINT(cert-err58-cpp)
    EventUtils::test_create_and_iterate([](std::string field_name, std::string name) {
        return Data{std::move(name), {std::move(field_name)}};
    });
}

TEST(EventDataTest, CreationAndIterationByField) { // NOLINT(cert-err58-cpp)
    EventUtils::test_create_and_iterate([](std::string field_name, std::string name) {
        Field field{std::move(field_name)};
        return Data{std::move(name), {field}};
    });
}

TEST(EventDataTest, GetByName) { // NOLINT(cert-err58-cpp)
    EventUtils::test_get_and_set([](Data eventData, std::string field_name, size_t _position, const std::string& _entry) {
        return eventData.getByName(std::move(field_name));
    }, "");
}

TEST(EventDataTest, SetForName) { // NOLINT(cert-err58-cpp)
    EventUtils::test_get_and_set([](Data eventData, std::string field_name, size_t _position, const std::string& entry) {
        eventData.setForName(field_name, entry);
        return eventData.getByName(std::move(field_name));
    }, "field");
}

TEST(EventDataTest, GetAtPosition) { // NOLINT(cert-err58-cpp)
    EventUtils::test_get_and_set([](Data eventData, const std::string& _field_name, size_t position, const std::string& _entry) {
        return eventData.getAtPosition(position);
    }, "");
}

TEST(EventDataTest, SetAtPosition) { // NOLINT(cert-err58-cpp)
    EventUtils::test_get_and_set([](Data eventData, std::string field_name, size_t position, const std::string& entry) {
        eventData.setAtPosition(position, entry);
        return eventData.getByName(std::move(field_name));
    }, "field");
}

TEST(EventDataTest, NumberOfFields) { // NOLINT(cert-err58-cpp)
    Data eventData{"", {""}};
    ASSERT_EQ(1, eventData.numberOfFields());
}

TEST(EventDataTest, GetName) { // NOLINT(cert-err58-cpp)
    Data eventData{"name", {""}};
    ASSERT_EQ("name", eventData.getName());
}