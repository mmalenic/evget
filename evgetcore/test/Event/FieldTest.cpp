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

// namespace EventTestUtils = TestUtils::EventTestUtils;
//
// TEST(FieldTest, ConstructorNoEntry) { // NOLINT(cert-err58-cpp)
//     EventTestUtils::fieldValueAndName(EvgetCore::Event::FieldDefinition{"name"}, "name", "");
// }
//
// TEST(FieldTest, ConstructorEntry) { // NOLINT(cert-err58-cpp)
//     EventTestUtils::fieldValueAndName(EvgetCore::Event::FieldDefinition{"name", "entry"}, "name", "entry");
// }
//
// TEST(FieldTest, ConstructorData) { // NOLINT(cert-err58-cpp)
//     auto field = EventTestUtils::createEntriesData("OuterField", "InnerData", "InnerField", "InnerEntry");
//
//     ASSERT_EQ("InnerField", field.begin()->begin()->getName());
//     ASSERT_EQ("InnerEntry", field.begin()->begin()->getName());
//     ASSERT_EQ("InnerData", field.begin()->getName());
//     ASSERT_EQ("OuterField", field.getName());
// }
//
// TEST(FieldTest, GetName) { // NOLINT(cert-err58-cpp)
//     EvgetCore::Event::FieldDefinition field{"name", "entry"};
//     ASSERT_EQ("name", field.getName());
// }
//
// TEST(FieldTest, GetEntry) { // NOLINT(cert-err58-cpp)
//     EvgetCore::Event::FieldDefinition field{"name", "entry"};
//     ASSERT_EQ("entry", field.getEntry());
// }
//
// TEST(FieldTest, IsEntry) { // NOLINT(cert-err58-cpp)
//     EvgetCore::Event::FieldDefinition field{"name", "entry"};
//     ASSERT_TRUE(field.isEntry());
//     ASSERT_FALSE(field.isData());
// }
//
// TEST(FieldTest, IsData) { // NOLINT(cert-err58-cpp)
//     EvgetCore::Event::FieldDefinition field = EventTestUtils::createEntriesData("OuterField", "InnerData",
//     "InnerField",
//                                                                       "InnerEntry");
//     ASSERT_TRUE(field.isData());
//     ASSERT_FALSE(field.isEntry());
// }
//
// TEST(FieldTest, CreateActionPress) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createAction(EvgetCore::Event::ButtonAction::Press);
//     ASSERT_EQ("Action", field.getName());
//     ASSERT_EQ("Press", field.getEntry());
// }
//
// TEST(FieldTest, CreateActionRelease) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createAction(EvgetCore::Event::ButtonAction::Release);
//     ASSERT_EQ("Action", field.getName());
//     ASSERT_EQ("Release", field.getEntry());
// }
//
// TEST(FieldTest, CreateActionRepeat) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createAction(EvgetCore::Event::ButtonAction::Repeat);
//     ASSERT_EQ("Action", field.getName());
//     ASSERT_EQ("Repeat", field.getEntry());
// }
//
// TEST(FieldTest, CreateCharacter) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createCharacter("a");
//     ASSERT_EQ("Character", field.getName());
//     ASSERT_EQ("a", field.getEntry());
// }
//
// TEST(FieldTest, CreateIdentifier) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createIdentifier(1);
//     ASSERT_EQ("Identifier", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }
//
// TEST(FieldTest, CreateName) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createName("name");
//     ASSERT_EQ("Name", field.getName());
//     ASSERT_EQ("name", field.getEntry());
// }
//
// TEST(FieldTest, CreateDateTime) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createDateTime({});
//     ASSERT_EQ("Timestamp", field.getName());
// }
//
// TEST(FieldTest, CreateDeviceTypeMouse) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createDeviceType(EvgetCore::Event::Device::Mouse);
//     ASSERT_EQ("Device", field.getName());
//     ASSERT_EQ("Mouse", field.getEntry());
// }
//
// TEST(FieldTest, CreateDeviceTypeKeyboard) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createDeviceType(EvgetCore::Event::Device::Keyboard);
//     ASSERT_EQ("Device", field.getName());
//     ASSERT_EQ("Keyboard", field.getEntry());
// }
//
// TEST(FieldTest, CreateDeviceTypeTouchpad) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createDeviceType(EvgetCore::Event::Device::Touchpad);
//     ASSERT_EQ("Device", field.getName());
//     ASSERT_EQ("Touchpad", field.getEntry());
// }
//
// TEST(FieldTest, CreateDeviceTypeTouchscreen) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createDeviceType(EvgetCore::Event::Device::Touchscreen);
//     ASSERT_EQ("Device", field.getName());
//     ASSERT_EQ("Touchscreen", field.getEntry());
// }
//
// TEST(FieldTest, CreateTime) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createTime(std::chrono::nanoseconds{1});
//     ASSERT_EQ("Time", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }
//
// TEST(FieldTest, CreatePositionX) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createPositionX(1);
//     ASSERT_EQ("PositionX", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }
//
// TEST(FieldTest, CreatePositionY) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createPositionY(1);
//     ASSERT_EQ("PositionY", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }
//
// TEST(FieldTest, CreateScrollUp) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createScroll(EvgetCore::Event::Direction::Up, 1);
//     ASSERT_EQ("ScrollUp", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }
//
// TEST(FieldTest, CreateScrollDown) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createScroll(EvgetCore::Event::Direction::Down, 1);
//     ASSERT_EQ("ScrollDown", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }
//
// TEST(FieldTest, CreateScrollLeft) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createScroll(EvgetCore::Event::Direction::Left, 1);
//     ASSERT_EQ("ScrollLeft", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }
//
// TEST(FieldTest, CreateScrollRight) { // NOLINT(cert-err58-cpp)
//     auto field = EvgetCore::Event::FieldDefinition::createScroll(EvgetCore::Event::Direction::Right, 1);
//     ASSERT_EQ("ScrollRight", field.getName());
//     ASSERT_EQ("1", field.getEntry());
// }