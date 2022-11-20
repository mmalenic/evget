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
#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Schema.h"

namespace TestUtils::EventTestUtils {
    EvgetCore::Event::Data createData();

    EvgetCore::Event::Schema<1, EvgetCore::Event::Schema<1>> createSchema();

    EvgetCore::Event::SchemaField::Field createEntriesData(const std::string& outerName, const std::string& innerDataName, const std::string& innerName, const std::string& innerEntry);

    void getAndSet(auto&& get, std::string entry) {
//        std::string field_name = "field";
//        EvgetCore::Event::Data eventData = createData("name", field_name, entry);
//        const auto& field = get(eventData, field_name, 0);
//
//        ASSERT_EQ(field_name, field.getName());
//        ASSERT_EQ(entry, field.getEntry());
    }

    void fieldValueAndName(const EvgetCore::Event::SchemaField::Field& field, const std::string& name, const std::string& expected);

    void event_entry_at(const EvgetCore::Event::Data& data, size_t position, const std::string& expected);
}

#endif //EVGET_TEST_TESTUTILS_INCLUDE_TESTUTILS_EVENT_EVENTTESTUTILS_H
