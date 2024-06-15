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

#include <X11/keysym.h>
#include <xorg/xserver-properties.h>

#include <array>

#include "EvgetX11TestUtils.h"
#include "evgetcore/Event/MouseClick.h"
#include "evgetx11/XEventSwitchPointerKey.h"

TEST(XEventSwitchPointerKeyTest, TestRefreshDevices) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock, xEventSwitch};

    XIValuatorClassInfo valuatorInfo{
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 0,
        .label = 0,
        .min = 0,
        .max = 0,
        .value = 0,
        .resolution = 0,
        .mode = 0};
    XIScrollClassInfo scrollInfo{
        .type = XIScrollClass,
        .sourceid = 0,
        .number = 0,
        .scroll_type = XIScrollTypeVertical,
        .increment = 0,
        .flags = 0};
    Atom labels[] = {1};
    unsigned char mask[] = {0};
    XISetMask(mask, 1);
    XIButtonClassInfo buttonInfo{
        .type = XIButtonClass,
        .sourceid = 0,
        .num_buttons = 1,
        .labels = labels,
        .state =
            XIButtonState{
                .mask_len = 1,
                .mask = mask,
            },
    };

    auto anyValuatorInfo = reinterpret_cast<XIAnyClassInfo*>(&valuatorInfo);
    auto anyScrollInfo = reinterpret_cast<XIAnyClassInfo*>(&scrollInfo);
    auto anyButtonInfo = reinterpret_cast<XIAnyClassInfo*>(&buttonInfo);
    XIAnyClassInfo* classes[] = {anyValuatorInfo, anyScrollInfo, anyButtonInfo};

    char name[] = "name";
    XIDeviceInfo xi2DeviceInfo{
        .deviceid = 0,
        .name = name,
        .use = 0,
        .attachment = 0,
        .enabled = true,
        .num_classes = 1,
        .classes = classes};

    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(
            testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char*)XI_MOUSE, [](void* _) { return 0;
            }})
        ));

    xEventSwitchPointerKey.refreshDevices(1, EvgetCore::Event::Device::Mouse, "name", xi2DeviceInfo);
}

TEST(XEventSwitchPointerKeyTest, TestButtonEvent) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock, xEventSwitch};

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto buttonClassInfo = EvgetX11TestUtils::createXIButtonClassInfo(labels, mask);

    std::array<XIAnyClassInfo*, 3> anyClassInfo = {reinterpret_cast<XIAnyClassInfo*>(&buttonClassInfo)};
    char name[] = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_ButtonPress, buttonMask, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    EXPECT_CALL(xWrapperMock, eventData)
        .WillOnce(testing::Return(
            testing::ByMove<EvgetX11::XEventPointer>({&xEvent.xcookie, [](XGenericEventCookie*) {}})
        ));
    EXPECT_CALL(xWrapperMock, nextEvent).WillOnce(testing::Return(testing::ByMove(EvgetX11TestUtils::createXEvent(deviceEvent))));
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

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, [](Time _) {
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

TEST(XEventSwitchCoreTest, TestKeyEvent) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock, xEventSwitch};

    std::array<XIAnyClassInfo*, 3> anyClassInfo = {};
    char name[] = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);

    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_KeyPress, buttonMask, valuatorMask, values);

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
    EXPECT_CALL(xWrapperMock, lookupCharacter).WillOnce([](const XIDeviceEvent& _event, KeySym& keySym) {
        keySym = XK_A;
        return "a";
    });

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(1, EvgetCore::Event::Device::Keyboard, "name", xiDeviceInfo);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, [](Time _) {
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

TEST(XEventSwitchCoreTest, TestMotionEvent) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock, xEventSwitch};

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
    .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>(
                    {(char*)AXIS_LABEL_PROP_ABS_X, [](void* _) { return 0; }}
                    )));

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(0, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, [](Time _) {
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

TEST(XEventSwitchCoreTest, TestScrollEvent) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock, xEventSwitch};

    auto valuatorClassInfo = EvgetX11TestUtils::createXIValuatorClassInfo();
    auto scrollClassInfo = EvgetX11TestUtils::createXIScrollClassInfo();

    std::array<XIAnyClassInfo*, 3> anyClassInfo = {
        reinterpret_cast<XIAnyClassInfo*>(&valuatorClassInfo),
        reinterpret_cast<XIAnyClassInfo*>(&scrollClassInfo)};
    char name[] = "name";
    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(anyClassInfo, name);
    xiDeviceInfo.deviceid = 0;

    std::array<unsigned char, 1> buttonMask = {1};
    std::array<unsigned char, 1> valuatorMask = {1};
    std::array<double, 1> values = {1};
    auto deviceEvent = EvgetX11TestUtils::createXIDeviceEvent(XI_Motion, buttonMask, valuatorMask, values);
    deviceEvent.deviceid = 0;

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
    .WillOnce(testing::Return(
        testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char*)XI_MOUSE, [](void* _) { return 0;
        }})
    ));
    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    xEventSwitchPointerKey.refreshDevices(0, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo);

    auto data = EvgetCore::Event::Data{};
    xEventSwitchPointerKey.switchOnEvent(inputEvent, data, [](Time _) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::MouseScroll);
    ASSERT_EQ(entries.at(0).data().at(0), "1");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "0");
    ASSERT_EQ(entries.at(0).data().at(12), EvgetCore::Event::fromDouble(-1.0));
    ASSERT_EQ(entries.at(0).data().at(13), "");
}