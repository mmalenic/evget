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
#include "evgetcore/Event/TableData.h"

TEST(TableDataTest, NoData) { // NOLINT(cert-err58-cpp)
    auto data = Event::TableData::TableDataBuilder{}.build();
    ASSERT_FALSE(data->getGenericData());
    ASSERT_FALSE(data->getSystemData());
}

TEST(TableDataTest, GenericData) { // NOLINT(cert-err58-cpp)
    auto data = Event::TableData::TableDataBuilder{}.genericData(std::make_unique<Event::Data>(Event::Data{"genericData", {}})).build();
    ASSERT_EQ("genericData", data->getGenericData()->getName());
    ASSERT_FALSE(data->getSystemData());
}

TEST(TableDataTest, SystemData) { // NOLINT(cert-err58-cpp)
    auto data = Event::TableData::TableDataBuilder{}.systemData(std::make_unique<Event::Data>(Event::Data{"systemData", {}})).build();
    ASSERT_FALSE(data->getGenericData());
    ASSERT_EQ("systemData", data->getSystemData()->getName());
}