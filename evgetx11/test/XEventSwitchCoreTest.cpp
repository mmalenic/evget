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
#include "evgetx11/XEventSwitchCore.h"
#include "EvgetX11TestUtils.h"
#include "evgetcore/Event/MouseClick.h"

TEST(XEventSwitchCore, TestRefreshDevices) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitchCore xEventSwitchCore{xWrapperMock};

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
    Atom labels[] = {1};
    unsigned char mask[] = {0};
    XISetMask(mask, 1);
    XIButtonClassInfo buttonInfo{
            .type = XIButtonClass,
            .sourceid = 0,
            .num_buttons = 1,
            .labels = labels,
            .state = XIButtonState {
                    .mask_len = 1,
                    .mask = mask,
            },
    };

    auto anyValuatorInfo = reinterpret_cast<XIAnyClassInfo *>(&valuatorInfo);
    auto anyScrollInfo = reinterpret_cast<XIAnyClassInfo *>(&scrollInfo);
    auto anyButtonInfo = reinterpret_cast<XIAnyClassInfo *>(&buttonInfo);
    XIAnyClassInfo* classes[] = {anyValuatorInfo, anyScrollInfo, anyButtonInfo};

    char name[] = "name";
    XIDeviceInfo xi2DeviceInfo{
            .deviceid = 0,
            .name = name,
            .use = 0,
            .attachment = 0,
            .enabled = true,
            .num_classes = 1,
            .classes = classes
    };

    EXPECT_CALL(xWrapperMock, atomName).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char *) XI_MOUSE, [](void *_){ return 0; }})));

    xEventSwitchCore.refreshDevices(0, EvgetCore::Event::Device::Mouse, "name", xi2DeviceInfo);
}

TEST(XEventSwitchCore, TestButtonEvent) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitchCore xEventSwitchCore{xWrapperMock};

    XIValuatorClassInfo valuatorInfo{
            .type = XIValuatorClass,
            .sourceid = 1,
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
            .sourceid = 1,
            .number = 0,
            .scroll_type = XIScrollTypeVertical,
            .increment = 0,
            .flags = 0
    };
    Atom labels[] = {1};
    unsigned char mask[] = {0};
    XISetMask(mask, 1);
    XIButtonClassInfo buttonInfo{
            .type = XIButtonClass,
            .sourceid = 1,
            .num_buttons = 1,
            .labels = labels,
            .state = XIButtonState {
                    .mask_len = 1,
                    .mask = mask,
            },
    };

    auto anyValuatorInfo = reinterpret_cast<XIAnyClassInfo *>(&valuatorInfo);
    auto anyScrollInfo = reinterpret_cast<XIAnyClassInfo *>(&scrollInfo);
    auto anyButtonInfo = reinterpret_cast<XIAnyClassInfo *>(&buttonInfo);
    XIAnyClassInfo* classes[] = {anyValuatorInfo, anyScrollInfo, anyButtonInfo};

    char name[] = "name";
    XIDeviceInfo xi2DeviceInfo{
            .deviceid = 1,
            .name = name,
            .use = 0,
            .attachment = 0,
            .enabled = true,
            .num_classes = 1,
            .classes = classes
    };

    auto buttonMask = nullptr;
    XISetMask(buttonMask, 1);

    auto valuatorMask = nullptr;
    XISetMask(valuatorMask, 1);
    double values[1] = {1};

    XIDeviceEvent deviceEvent{
            .type = 0,
            .serial = 0,
            .send_event = 0,
            .display = nullptr,
            .extension = 0,
            .evtype = 0,
            .time = 0,
            .deviceid = 1,
            .sourceid = 1,
            .detail = 0,
            .root = 0,
            .event = 0,
            .child = 0,
            .root_x = 1,
            .root_y = 1,
            .event_x = 0,
            .event_y = 0,
            .flags = 0,
            .buttons = {
                    .mask_len = 1,
                    .mask = buttonMask,
            },
            .valuators = {
                    .mask_len = 1,
                    .mask = valuatorMask,
                    .values = values
            },
            .mods = {0, 0, 0, 0},
            .group = {0, 0, 0, 0},
    };

    XGenericEventCookie cookie{
            .type = GenericEvent,
            .serial = 0,
            .send_event = false,
            .display = nullptr,
            .extension = 0,
            .evtype = XI_ButtonPress,
            .cookie = 0,
            .data = &deviceEvent,
    };
    XEvent event{
            .xcookie = cookie
    };

    EXPECT_CALL(xWrapperMock, nextEvent).WillOnce(testing::Return(testing::ByMove(event)));
    EXPECT_CALL(xWrapperMock, atomName).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char *) XI_MOUSE, [](void *_){ return 0; }})));

    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    EvgetX11::XEventSwitch::EventData eventData{};
    xEventSwitchCore.refreshDevices(0, EvgetCore::Event::Device::Mouse, "name", xi2DeviceInfo);
    xEventSwitchCore.switchOnEvent(inputEvent, std::chrono::nanoseconds{1}, eventData);

    auto genericData = eventData.at(0);

    ASSERT_EQ(genericData.getFieldAt(0).getEntry(), "1");
    ASSERT_EQ(genericData.getFieldAt(2).getEntry(), "0");
    ASSERT_EQ(genericData.getFieldAt(3).getEntry(), "0");
    ASSERT_EQ(genericData.getFieldAt(4).getEntry(), "0");
    ASSERT_EQ(genericData.getFieldAt(5).getEntry(), "Press");
    ASSERT_EQ(genericData.getFieldAt(5).getEntry(), "name");
}