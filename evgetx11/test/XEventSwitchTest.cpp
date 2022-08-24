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
//

#include <gtest/gtest.h>
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
    auto mask = nullptr;
    XISetMask(mask, 1);
    double values[1] = {1};

    auto valuatorState = XIValuatorState {
         .mask_len = 1,
         .mask = mask,
         .values = values
    };

    auto valuators = EvgetX11TestUtils::XEventSwitchMock::getValuators(valuatorState);
    std::map<int, int> expected{};
    expected[1] = 1;

    ASSERT_EQ(valuators, expected);
}


TEST(XEventSwitchTest, CreateButtonEntries) { // NOLINT(cert-err58-cpp)
    auto mask = nullptr;
    XISetMask(mask, 1);

    auto event = XIDeviceEvent {
        .buttons = XIButtonState{
                .mask_len = 1,
                .mask = mask,
        }
    };

    auto buttons = EvgetX11TestUtils::XEventSwitchMock::createButtonEntries(event);
    ASSERT_EQ(buttons.at(0).getName(), "ButtonState");
    ASSERT_EQ(buttons.at(0).getFieldAt(0).getName(), "ButtonActive");
    ASSERT_EQ(buttons.at(0).getFieldAt(0).getEntry(), "1");
}