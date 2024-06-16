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

#include "EvgetX11TestUtils.h"
#include "evgetx11/XEventSwitch.h"

TEST(XEventSwitchTest, RefreshDevices) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    ASSERT_FALSE(xEventSwitch.hasDevice(1));

    xEventSwitch.refreshDevices(1, EvgetCore::Event::Device::Mouse, "name", {});
    ASSERT_TRUE(xEventSwitch.hasDevice(1));
}

TEST(XEventSwitchTest, GetDevice) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    ASSERT_THROW(xEventSwitch.getDevice(1), std::out_of_range);

    xEventSwitch.refreshDevices(1, EvgetCore::Event::Device::Mouse, "name", {});
    ASSERT_EQ(xEventSwitch.getDevice(1), EvgetCore::Event::Device::Mouse);
}

TEST(XEventSwitchTest, GetButtonName) {  // NOLINT(cert-err58-cpp)
    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_ButtonPress, buttonMask, valuatorMask, values);

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto buttonClassInfo = EvgetX11TestUtils::createXIButtonClassInfo(labels, mask);

    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};

    ASSERT_THROW(xEventSwitch.getButtonName(1, 0), std::out_of_range);

    EXPECT_CALL(xWrapperMock, getDeviceButtonMapping)
    .WillOnce(testing::Return(
        testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1))
    ));
    EXPECT_CALL(xWrapperMock, atomName)
    .WillOnce(testing::Return(
        testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char*)XI_MOUSE, [](void* _) { return 0;
        }})
    ));

    xEventSwitch.refreshDevices(1, EvgetCore::Event::Device::Mouse, "name", {});
    xEventSwitch.setButtonMap(buttonClassInfo, 1);
    ASSERT_EQ(xEventSwitch.getButtonName(1, 0), "MOUSE");
}

TEST(XEventSwitchPointerTest, TestAddButtonEvent) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};

    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_ButtonPress, buttonMask, valuatorMask, values);
    deviceEvent.deviceid = 1;

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto buttonClassInfo = EvgetX11TestUtils::createXIButtonClassInfo(labels, mask);

    EXPECT_CALL(xWrapperMock, getDeviceButtonMapping)
    .WillOnce(testing::Return(
        testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1))
    ));
    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(
            testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char*)XI_MOUSE, [](void* _) { return 0;
            }})
        ));
    EXPECT_CALL(xWrapperMock, getActiveWindow)
    .WillOnce(testing::Return(
        testing::ByMove<std::optional<Window> >({std::nullopt})
    ));
    EXPECT_CALL(xWrapperMock, getFocusWindow)
    .WillOnce(testing::Return(
        testing::ByMove<std::optional<Window> >({std::nullopt})
    ));

    xEventSwitch.setButtonMap(buttonClassInfo, 1);
    xEventSwitch.refreshDevices(1, EvgetCore::Event::Device::Mouse, "name", {});

    EvgetCore::Event::Data data{};
    xEventSwitch.addButtonEvent(deviceEvent, EvgetCore::Event::Timestamp{}, data, EvgetCore::Event::ButtonAction::Press, 0, [](Time _) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::MouseClick);
    ASSERT_EQ(entries.at(0).data().at(0), "1");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "0");
    ASSERT_EQ(entries.at(0).data().at(12), "0");
    ASSERT_EQ(entries.at(0).data().at(13), "MOUSE");
    ASSERT_EQ(entries.at(0).data().at(14), "0");
}

TEST(XEventSwitchPointerTest, TestAddMotionEvent) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};

    auto valuatorClassInfo = EvgetX11TestUtils::createXIValuatorClassInfo();
    valuatorClassInfo.number = 0;

    std::array<XIAnyClassInfo*, 3> anyClassInfo = {reinterpret_cast<XIAnyClassInfo*>(&valuatorClassInfo)};
    char name[] = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);
    xiDeviceInfo.deviceid = 0;

    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_Motion, buttonMask, valuatorMask, values);
    deviceEvent.deviceid = 0;

    EXPECT_CALL(xWrapperMock, getActiveWindow)
    .WillOnce(testing::Return(
        testing::ByMove<std::optional<Window> >({std::nullopt})
    ));
    EXPECT_CALL(xWrapperMock, getFocusWindow)
    .WillOnce(testing::Return(
        testing::ByMove<std::optional<Window> >({std::nullopt})
    ));

    xEventSwitch.refreshDevices(0, EvgetCore::Event::Device::Mouse, "name", {});

    EvgetCore::Event::Data data{};
    xEventSwitch.addMotionEvent(deviceEvent, EvgetCore::Event::Timestamp{}, data, [](Time _) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::MouseMove);
    ASSERT_EQ(entries.at(0).data().at(0), "1");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "0");
}
