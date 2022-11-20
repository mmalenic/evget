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
#include "EventTestUtils.h"

#include <utility>
#include "evgetcore/UnsupportedOperationException.h"

namespace EventUtils = TestUtils::EventTestUtils;

TEST(EventDataTest, CreateAndIterate) { // NOLINT(cert-err58-cpp)
    EvgetCore::Event::Data data{"name"};
    data.addField("field");

    auto n = 0;
    for (auto i{data.begin()}; i != data.end(); i++, n++) {
            ASSERT_EQ(*i, "field");
    }
    ASSERT_EQ(n, 1);
}

TEST(EventDataTest, GetName) { // NOLINT(cert-err58-cpp)
    EvgetCore::Event::Data eventData{"name"};
    ASSERT_EQ("name", eventData.getName());
}