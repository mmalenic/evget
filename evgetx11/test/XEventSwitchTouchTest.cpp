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

#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Device.h"
#include "evgetcore/Event/Entry.h"
#include "evgetcore/Event/Schema.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XEventSwitchTouch.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XWrapper.h"
#include "utils/EvgetX11TestUtils.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(XEventSwitchTouchTest, TestTouchBegin) {  // NOLINT(readability-function-cognitive-complexity)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchTouch xEventSwitchTouch{};  // NOLINT(misc-const-correctness)

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto buttonClassInfo = EvgetX11TestUtils::createXIButtonClassInfo(labels, mask);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> anyClassInfo = {reinterpret_cast<XIAnyClassInfo*>(&buttonClassInfo)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    char name[] = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawTouchBegin, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    EXPECT_CALL(xWrapperMock, eventData)
        .WillOnce(
            testing::Return(testing::ByMove<EvgetX11::XEventPointer>({&xEvent.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(xWrapperMock, nextEvent)
        .WillOnce(testing::Return(testing::ByMove(EvgetX11TestUtils::createXEvent(deviceEvent))));
    EXPECT_CALL(xWrapperMock, getDeviceButtonMapping)
        .WillOnce(
            testing::Return(testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1)))
        );
    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void* _) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(xWrapperMock, getActiveWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(xWrapperMock, getFocusWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(xWrapperMock, query_pointer).WillRepeatedly([]() {
        return EvgetX11TestUtils::create_pointer_result();
    });

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitch.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchTouch.switchOnEvent(inputEvent, data, xEventSwitch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::MouseMove);
    ASSERT_EQ(entries.at(0).data().at(0), "1");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "0");

    ASSERT_EQ(entries.at(1).type(), EvgetCore::Event::EntryType::MouseClick);
    ASSERT_EQ(entries.at(1).data().at(0), "1");
    ASSERT_EQ(entries.at(1).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(1).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(1).data().at(4), "name");
    ASSERT_EQ(entries.at(1).data().at(11), "0");
    ASSERT_EQ(entries.at(1).data().at(12), "0");
    ASSERT_EQ(entries.at(1).data().at(13), "TOUCHSCREEN");
    ASSERT_EQ(entries.at(1).data().at(14), "0");
}

TEST(XEventSwitchTouchTest, TestTouchUpdate) {
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchTouch xEventSwitchTouch{};  // NOLINT(misc-const-correctness)

    auto valuatorClassInfo = EvgetX11TestUtils::createXIValuatorClassInfo();
    valuatorClassInfo.number = 0;

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> anyClassInfo = {reinterpret_cast<XIAnyClassInfo*>(&valuatorClassInfo)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    char name[] = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawTouchUpdate, valuatorMask, values);

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
    EXPECT_CALL(xWrapperMock, query_pointer).WillRepeatedly([]() {
        return EvgetX11TestUtils::create_pointer_result();
    });

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitch.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchTouch.switchOnEvent(inputEvent, data, xEventSwitch, [](Time) {
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

TEST(XEventSwitchTouchTest, TestTouchEnd) {  // NOLINT(readability-function-cognitive-complexity)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchTouch xEventSwitchTouch{};  // NOLINT(misc-const-correctness)

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto buttonClassInfo = EvgetX11TestUtils::createXIButtonClassInfo(labels, mask);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> anyClassInfo = {reinterpret_cast<XIAnyClassInfo*>(&buttonClassInfo)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    char name[] = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawTouchEnd, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    EXPECT_CALL(xWrapperMock, eventData)
        .WillOnce(
            testing::Return(testing::ByMove<EvgetX11::XEventPointer>({&xEvent.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(xWrapperMock, nextEvent)
        .WillOnce(testing::Return(testing::ByMove(EvgetX11TestUtils::createXEvent(deviceEvent))));
    EXPECT_CALL(xWrapperMock, getDeviceButtonMapping)
        .WillOnce(
            testing::Return(testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1)))
        );
    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void* _) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(xWrapperMock, getActiveWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(xWrapperMock, getFocusWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(xWrapperMock, query_pointer).WillRepeatedly([]() {
        return EvgetX11TestUtils::create_pointer_result();
    });

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitch.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchTouch.switchOnEvent(inputEvent, data, xEventSwitch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::MouseMove);
    ASSERT_EQ(entries.at(0).data().at(0), "1");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "0");

    ASSERT_EQ(entries.at(1).type(), EvgetCore::Event::EntryType::MouseClick);
    ASSERT_EQ(entries.at(1).data().at(0), "1");
    ASSERT_EQ(entries.at(1).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(1).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(1).data().at(4), "name");
    ASSERT_EQ(entries.at(1).data().at(11), "0");
    ASSERT_EQ(entries.at(1).data().at(12), "0");
    ASSERT_EQ(entries.at(1).data().at(13), "TOUCHSCREEN");
    ASSERT_EQ(entries.at(1).data().at(14), "1");
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
