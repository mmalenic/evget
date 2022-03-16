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
#include "evgetcore/Event/AbstractData.h"
#include "EventTestUtils.h"

#include <utility>
#include "evgetcore/UnsupportedOperationException.h"

namespace EventUtils = TestUtils::EventTestUtils;

TEST(EventDataTest, CreationAndIterationByField) { // NOLINT(cert-err58-cpp)
    EventUtils::createAndIterate(
        [](std::string field_name, std::string name, std::string entry) {
            std::vector<std::unique_ptr<Event::AbstractField>> fields{};
            fields.emplace_back(std::make_unique<Event::Field>(std::move(field_name), std::move(entry)));
            return Event::Data{std::move(name), std::move(fields)};
        }
    );
}

TEST(EventDataTest, GetByName) { // NOLINT(cert-err58-cpp)
    EventUtils::getAndSet(
        [](Event::Data& eventData, std::string field_name, size_t _position) -> const auto& {
            return eventData.getByName(std::move(field_name));
        }, ""
    );
}

TEST(EventDataTest, GetAtPosition) { // NOLINT(cert-err58-cpp)
    EventUtils::getAndSet(
        [](Event::Data& eventData, const std::string& _field_name, size_t position) -> const auto& {
            return eventData.getAtPosition(position);
        }, ""
    );
}

TEST(EventDataTest, GetName) { // NOLINT(cert-err58-cpp)
    Event::Data eventData{"name", EventUtils::allocateFields("", "")};
    ASSERT_EQ("name", eventData.getName());
}