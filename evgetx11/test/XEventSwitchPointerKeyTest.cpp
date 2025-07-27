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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <string>

#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Device.h"
#include "evgetcore/Event/Entry.h"
#include "evgetcore/Event/Schema.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XEventSwitchPointerKey.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XWrapper.h"
#include "utils/EvgetX11TestUtils.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(XEventSwitchPointerKeyTest, TestRefreshDevices) {
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock};

    XIValuatorClassInfo valuatorInfo{
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 0,
        .label = 0,
        .min = 0,
        .max = 0,
        .value = 0,
        .resolution = 0,
        .mode = 0
    };
    XIScrollClassInfo scrollInfo{
        .type = XIScrollClass,
        .sourceid = 0,
        .number = 0,
        .scroll_type = XIScrollTypeVertical,
        .increment = 0,
        .flags = 0
    };
    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {0};
    XISetMask(mask.data(), 1);
    XIButtonClassInfo buttonInfo{
        .type = XIButtonClass,
        .sourceid = 0,
        .num_buttons = 1,
        .labels = labels.data(),
        .state = XIButtonState{
            .mask_len = 1,
            .mask = mask.data(),
        },
    };

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* anyValuatorInfo = reinterpret_cast<XIAnyClassInfo*>(&valuatorInfo);
    auto* anyScrollInfo = reinterpret_cast<XIAnyClassInfo*>(&scrollInfo);
    auto* anyButtonInfo = reinterpret_cast<XIAnyClassInfo*>(&buttonInfo);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array classes = {anyValuatorInfo, anyScrollInfo, anyButtonInfo};

    std::string name = "name";
    const XIDeviceInfo xi2DeviceInfo{
        .deviceid = 0,
        .name = name.data(),
        .use = 0,
        .attachment = 0,
        .enabled = True,
        .num_classes = 1,
        .classes = classes.data()
    };

    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void* _) {
                                                                                                  return 0;
                                                                                              }})));

    xEventSwitchPointerKey.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xi2DeviceInfo, xEventSwitch);
}

TEST(XEventSwitchPointerKeyTest, TestButtonEvent) {  // NOLINT(readability-function-cognitive-complexity)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock};

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto buttonClassInfo = EvgetX11TestUtils::createXIButtonClassInfo(labels, mask);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> anyClassInfo = {reinterpret_cast<XIAnyClassInfo*>(&buttonClassInfo)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawButtonPress, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    EvgetX11TestUtils::set_x_wrapper_event_mocks(xWrapperMock, deviceEvent, xEvent);

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo, xEventSwitch);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, xEventSwitch, [](Time) {
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

TEST(XEventSwitchCoreTest, TestKeyEvent) {  // NOLINT(readability-function-cognitive-complexity)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock};

    std::array<XIAnyClassInfo*, 3> anyClassInfo = {};
    std::string name = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawKeyPress, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    EvgetX11TestUtils::set_x_wrapper_key_mocks(xWrapperMock, deviceEvent, xEvent);
    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(1, 1, EvgetCore::Event::Device::Keyboard, "name", xiDeviceInfo, xEventSwitch);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, xEventSwitch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::Key);
    ASSERT_EQ(entries.at(0).data().at(0), "1");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "1");
    ASSERT_EQ(entries.at(0).data().at(12), "0");
    ASSERT_EQ(entries.at(0).data().at(13), "A");
    ASSERT_EQ(entries.at(0).data().at(14), "0");
    ASSERT_EQ(entries.at(0).data().at(15), "a");
}

TEST(XEventSwitchCoreTest, TestMotionEvent) {
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock};

    auto valuatorClassInfo = EvgetX11TestUtils::createXIValuatorClassInfo();
    valuatorClassInfo.number = 0;

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> anyClassInfo = {reinterpret_cast<XIAnyClassInfo*>(&valuatorClassInfo)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawMotion, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    EXPECT_CALL(xWrapperMock, eventData)
        .WillOnce(
            testing::Return(testing::ByMove<EvgetX11::XEventPointer>({&xEvent.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(xWrapperMock, nextEvent)
        .WillOnce(testing::Return(testing::ByMove(EvgetX11TestUtils::createXEvent(deviceEvent))));
    EXPECT_CALL(xWrapperMock, getActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, getFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void* _) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(xWrapperMock, query_pointer).WillRepeatedly([]() {
        return EvgetX11TestUtils::create_pointer_result();
    });

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo, xEventSwitch);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, xEventSwitch, [](Time) {
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

TEST(XEventSwitchCoreTest, TestScrollEvent) {  // NOLINT(readability-function-cognitive-complexity)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock};

    auto valuatorClassInfo = EvgetX11TestUtils::createXIValuatorClassInfo();
    auto scrollClassInfo = EvgetX11TestUtils::createXIScrollClassInfo();

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> anyClassInfo = {
        reinterpret_cast<XIAnyClassInfo*>(&valuatorClassInfo),
        reinterpret_cast<XIAnyClassInfo*>(&scrollClassInfo)
    };
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawMotion, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    EXPECT_CALL(xWrapperMock, eventData)
        .WillOnce(
            testing::Return(testing::ByMove<EvgetX11::XEventPointer>({&xEvent.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(xWrapperMock, nextEvent)
        .WillOnce(testing::Return(testing::ByMove(EvgetX11TestUtils::createXEvent(deviceEvent))));
    EXPECT_CALL(xWrapperMock, getActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, getFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void* _) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(xWrapperMock, query_pointer).WillRepeatedly([]() {
        return EvgetX11TestUtils::create_pointer_result();
    });

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo, xEventSwitch);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, xEventSwitch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::MouseScroll);
    ASSERT_EQ(entries.at(0).data().at(0), "1");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "0");
    ASSERT_EQ(entries.at(0).data().at(12), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(13), "");
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
