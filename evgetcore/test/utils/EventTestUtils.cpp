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

// void TestUtils::EventTestUtils::fieldValueAndName(
//     const EvgetCore::Event::SchemaField::Field& field,
//     const std::string& name,
//     const std::string& expected
//) {
//     //    ASSERT_EQ(name, field.getName());
//     //    ASSERT_EQ(expected, field.getEntry());
// }
//
// EvgetCore::Event::Data TestUtils::EventTestUtils::createData() {
//     EvgetCore::Event::Data data{"name"};
//     data.addField("field");
//
//     EvgetCore::Event::Data firstContained{"contained"};
//     firstContained.addField("firstField");
//     EvgetCore::Event::Data secondContained{"contained"};
//     secondContained.addField("secondField");
//
//     //    data.contains(firstContained);
//     //    data.contains(secondContained);
//     //    data.containsUnique(firstContained);
//     //    data.containsUnique(secondContained);
//
//     return data;
// }
//
// EvgetCore::Event::Schema<1, EvgetCore::Event::Schema<1>> TestUtils::EventTestUtils::createSchema() {
//     EvgetCore::Event::Schema<1> relationSchema{
//         "Relation",
//         {EvgetCore::Event::SchemaField::Field{"RelationField", EvgetCore::Event::DataType::String}}};
//     return {
//         "Name",
//         {EvgetCore::Event::SchemaField::Field{"FieldDefinition", EvgetCore::Event::DataType::String}},
//         {relationSchema, false}};
// }
//
// EvgetCore::Event::SchemaField::Field TestUtils::EventTestUtils::createEntriesData(
//     const std::string& outerName,
//     const std::string& innerDataName,
//     const std::string& innerName,
//     const std::string& innerEntry
//) {
//     //    EvgetCore::Event::FieldDefinition::Entries entries{};
//     //    EvgetCore::Event::Data data = TestUtils::EventTestUtils::createData(innerDataName, innerName, innerEntry);
//
//     //    entries.emplace_back(data);
//     //    return EvgetCore::Event::FieldDefinition{outerName, std::move(entries)};
// }
//
// void TestUtils::EventTestUtils::event_entry_at(
//     const EvgetCore::Event::Data& data,
//     size_t position,
//     const std::string& expected
//) {
//     //    ASSERT_EQ(data.getFieldAt(position).getEntry(), expected);
// }