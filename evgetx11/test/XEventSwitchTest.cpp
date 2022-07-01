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
    std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};
    std::unique_ptr<EvgetCore::Event::AbstractData> genericData = std::make_unique<EvgetCore::Event::Data>("Test", std::move(fields));

    EvgetX11::XEventSwitch::EventData data{};
    EvgetX11::XEventSwitch::addTableData(data, std::move(genericData), nullptr);

    ASSERT_EQ(data.at(0)->getGenericData()->getName(), "Test");
    ASSERT_EQ(data.at(0)->getSystemData(), nullptr);
}

TEST(XEventSwitchTest, ContainsDevice) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    ASSERT_FALSE(eventSwitch.containsDevice(1));
    eventSwitch.setDevice(1, EvgetCore::Event::Common::Device::Mouse);
    ASSERT_FALSE(eventSwitch.containsDevice(1));
}

TEST(XEventSwitchTest, GetDevice) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    eventSwitch.setDevice(1, EvgetCore::Event::Common::Device::Mouse);
    ASSERT_EQ(eventSwitch.getDevice(1), EvgetCore::Event::Common::Device::Mouse);
}

TEST(XEventSwitchTest, GetNameFromId) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    eventSwitch.setNameFromId(1, "Name");
    ASSERT_EQ(eventSwitch.getNameFromId(1), "Name");
}

TEST(XEventSwitchTest, GetEvtypeName) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XEventSwitchMock eventSwitch{};
    eventSwitch.setEvtypeName(1, "Evtype");
    ASSERT_EQ(eventSwitch.getNameFromId(1), "Evtype");
}