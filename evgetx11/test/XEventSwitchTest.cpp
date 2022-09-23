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
#include <array>
#include "evgetx11/XEventSwitch.h"
#include "EvgetX11TestUtils.h"

TEST(XEventSwitchTest, TestAddTableData) { // NOLINT(cert-err58-cpp)
    EvgetX11::XEventSwitch::EventData data{};
    EvgetX11::XEventSwitch::addTableData(data, EvgetCore::Event::Data{"Test"}, EvgetCore::Event::Data{"Test"});

    ASSERT_EQ(data.at(0).begin()->getName(), "Test");
}

TEST(XEventSwitchTest, ContainsDevice) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    ASSERT_FALSE(eventSwitch.containsDevice(1));
    eventSwitch.setDevice(1, EvgetCore::Event::Device::Mouse);
    ASSERT_FALSE(eventSwitch.containsDevice(1));
}

TEST(XEventSwitchTest, Device) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    ASSERT_THROW(eventSwitch.getDevice(1), std::out_of_range);
    eventSwitch.setDevice(1, EvgetCore::Event::Device::Mouse);
    ASSERT_EQ(eventSwitch.getDevice(1), EvgetCore::Event::Device::Mouse);
}

TEST(XEventSwitchTest, NameFromId) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    ASSERT_THROW(eventSwitch.getNameFromId(1), std::out_of_range);
    eventSwitch.setNameFromId(1, "Name");
    ASSERT_EQ(eventSwitch.getNameFromId(1), "Name");
}

TEST(XEventSwitchTest, EvtypeName) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    ASSERT_THROW(eventSwitch.getEvtypeName(1), std::out_of_range);
    eventSwitch.setEvtypeName(1, "Evtype");
    ASSERT_EQ(eventSwitch.getEvtypeName(1), "Evtype");
}

TEST(XEventSwitchTest, FormatValue) { // NOLINT(cert-err58-cpp)
    ASSERT_EQ(EvgetX11TestUtils::XEventSwitchMock::formatValue(0), "");
    ASSERT_EQ(EvgetX11TestUtils::XEventSwitchMock::formatValue(1), "1");
}

TEST(XEventSwitchTest, GetValuators) { // NOLINT(cert-err58-cpp)
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto valuatorState = EvgetX11TestUtils::createXIValuatorState(valuatorMask, values);

    auto valuators = EvgetX11TestUtils::XEventSwitchMock::getValuators(valuatorState);
    std::map<int, int> expected{};
    expected[1] = 1;

    ASSERT_EQ(valuators, expected);
}

TEST(XEventSwitchTest, CreateButtonEntries) { // NOLINT(cert-err58-cpp)
    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_ButtonPress, buttonMask, valuatorMask, values);

    auto buttons = EvgetX11TestUtils::XEventSwitchMock::createButtonEntries(deviceEvent);
    ASSERT_EQ(buttons.at(0).getName(), "ButtonState");
    ASSERT_EQ(buttons.at(0).getFieldAt(0).getName(), "ButtonActive");
    ASSERT_EQ(buttons.at(0).getFieldAt(0).getEntry(), "1");
}

TEST(XEventSwitchTest, CreateValuatorEntries) { // NOLINT(cert-err58-cpp)
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto valuatorState = EvgetX11TestUtils::createXIValuatorState(valuatorMask, values);

    auto buttons = EvgetX11TestUtils::XEventSwitchMock::createValuatorEntries(valuatorState);
    ASSERT_EQ(buttons.at(0).getName(), "Valuators");

    ASSERT_EQ(buttons.at(0).getFieldAt(0).getName(), "Valuator");
    ASSERT_EQ(buttons.at(0).getFieldAt(0).getEntry(), "1");

    ASSERT_EQ(buttons.at(0).getFieldAt(1).getName(), "Value");
    ASSERT_EQ(buttons.at(0).getFieldAt(1).getEntry(), "1");
}

TEST(XEventSwitchTest, CreateSystemData) { // NOLINT(cert-err58-cpp)
    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_ButtonPress, buttonMask, valuatorMask, values);

    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};

    eventSwitch.setDevice(0, EvgetCore::Event::Device::Mouse);
    eventSwitch.setEvtypeName(0, "Name");
    eventSwitch.setNameFromId(0, "Name");

    auto data = eventSwitch.createSystemData(deviceEvent, "Name");

    ASSERT_EQ(data.getName(), "Name");

    ASSERT_EQ(data.getFieldAt(0).getName(), "DeviceName");
    ASSERT_EQ(data.getFieldAt(0).getEntry(), "Name");

    ASSERT_EQ(data.getFieldAt(1).getName(), "EventTypeId");
    ASSERT_EQ(data.getFieldAt(1).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(2).getName(), "EventTypeName");
    ASSERT_EQ(data.getFieldAt(2).getEntry(), "Name");

    ASSERT_EQ(data.getFieldAt(3).getName(), "XInputTime");
    ASSERT_EQ(data.getFieldAt(3).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(4).getName(), "DeviceId");
    ASSERT_EQ(data.getFieldAt(4).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(5).getName(), "SourceId");
    ASSERT_EQ(data.getFieldAt(5).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(6).getName(), "Flags");
    ASSERT_EQ(data.getFieldAt(6).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(7).getName(), "ButtonState");
    ASSERT_EQ(data.getFieldAt(7).getEntryAt(0).getName(), "ButtonState");
    ASSERT_EQ(data.getFieldAt(7).getEntryAt(0).getFieldAt(0).getName(), "ButtonActive");
    ASSERT_EQ(data.getFieldAt(7).getEntryAt(0).getFieldAt(0).getEntry(), "1");

    ASSERT_EQ(data.getFieldAt(8).getName(), "Valuators");
    ASSERT_EQ(data.getFieldAt(8).getEntryAt(0).getName(), "Valuators");
    ASSERT_EQ(data.getFieldAt(8).getEntryAt(0).getFieldAt(0).getName(), "Valuator");
    ASSERT_EQ(data.getFieldAt(8).getEntryAt(0).getFieldAt(0).getEntry(), "1");
    ASSERT_EQ(data.getFieldAt(8).getEntryAt(1).getFieldAt(1).getName(), "Value");
    ASSERT_EQ(data.getFieldAt(8).getEntryAt(1).getFieldAt(1).getEntry(), "1");

    ASSERT_EQ(data.getFieldAt(9).getName(), "ModifiersBase");
    ASSERT_EQ(data.getFieldAt(9).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(10).getName(), "ModifiersEffective");
    ASSERT_EQ(data.getFieldAt(10).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(11).getName(), "ModifiersLatched");
    ASSERT_EQ(data.getFieldAt(11).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(12).getName(), "ModifiersLocked");
    ASSERT_EQ(data.getFieldAt(12).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(13).getName(), "GroupBase");
    ASSERT_EQ(data.getFieldAt(13).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(14).getName(), "GroupEffective");
    ASSERT_EQ(data.getFieldAt(14).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(15).getName(), "GroupLatched");
    ASSERT_EQ(data.getFieldAt(15).getEntry(), "0");

    ASSERT_EQ(data.getFieldAt(16).getName(), "GroupLocked");
    ASSERT_EQ(data.getFieldAt(16).getEntry(), "0");
}