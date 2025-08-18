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

#include "utils/EvgetX11TestUtils.h"

#include <gmock/gmock.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysymdef.h>

#include <array>
#include <memory>
#include <optional>
#include <span>

#include "evgetx11/XWrapper.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
XIValuatorClassInfo EvgetX11TestUtils::createXIValuatorClassInfo() {
    return {
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 1,
        .label = 1,
        .min = 0,
        .max = 0,
        .value = 1,
        .resolution = 1,
        .mode = XIModeAbsolute
    };
}

XIScrollClassInfo EvgetX11TestUtils::createXIScrollClassInfo() {
    return {
        .type = XIScrollClass,
        .sourceid = 1,
        .number = 0,
        .scroll_type = XIScrollTypeVertical,
        .increment = 1,
        .flags = 0
    };
}

XIButtonClassInfo
EvgetX11TestUtils::createXIButtonClassInfo(std::array<Atom, 1>& labels, std::array<unsigned char, 1>& mask) {
    return {
        .type = XIButtonClass,
        .sourceid = 1,
        .num_buttons = static_cast<int>(labels.size()),
        .labels = labels.data(),
        .state = {
            .mask_len = static_cast<int>(mask.size()),
            .mask = mask.data(),
        },
    };
}

XIDeviceInfo EvgetX11TestUtils::createXIDeviceInfo(std::array<XIAnyClassInfo*, 3>& info, std::span<char> name) {
    return {
        .deviceid = 1,
        .name = name.data(),
        .use = XIMasterPointer,
        .attachment = 0,
        .enabled = True,
        .num_classes = static_cast<int>(info.size()),
        .classes = info.data()
    };
}

XDeviceInfo EvgetX11TestUtils::createXDeviceInfo() {
    return {
        .id = 1,
        .type = 1,
        .name = nullptr,
        .num_classes = 0,
        .use = IsXExtensionPointer,
        .inputclassinfo = nullptr
    };
}

XIRawEvent EvgetX11TestUtils::createXIRawEvent(
    int evtype,
    std::array<unsigned char, 1>& valuatorMask,
    std::array<double, 1>& values
) {
    return {
        .type = GenericEvent,
        .serial = 1,
        .send_event = False,
        .display = nullptr,
        .extension = 0,
        .evtype = evtype,
        .time = 1,
        .deviceid = 1,
        .sourceid = 1,
        .detail = 0,
        .flags = 0,
        .valuators = createXIValuatorState(valuatorMask, values),
    };
}

XEvent EvgetX11TestUtils::createXEvent(XIRawEvent& event) {
    return {
        .xcookie = {
            .type = GenericEvent,
            .serial = 0,
            .send_event = False,
            .display = nullptr,
            .extension = 0,
            .evtype = event.evtype,
            .cookie = 0,
            .data = &event,
        }
    };
}

XIValuatorState
EvgetX11TestUtils::createXIValuatorState(std::array<unsigned char, 1>& valuatorMask, std::array<double, 1>& values) {
    return {.mask_len = static_cast<int>(valuatorMask.size()), .mask = valuatorMask.data(), .values = values.data()};
}

EvgetX11::QueryPointerResult EvgetX11TestUtils::create_pointer_result() {
    return EvgetX11::QueryPointerResult{
        .root_x = 1,
        .root_y = 1,
        .button_mask = {nullptr, XFree},
        .modifier_state =
            XIModifierState{
                .base = 0,
                .latched = 0,
                .locked = 0,
                .effective = 0,
            },
        .group_state =
            XIGroupState{
                .base = 0,
                .latched = 0,
                .locked = 0,
                .effective = 0,
            },
        .screen_number = 0
    };
}

void EvgetX11TestUtils::set_x_wrapper_event_mocks(
    EvgetX11TestUtils::XWrapperMock& xWrapperMock,
    XIRawEvent& device_event,
    XEvent& xEvent
) {
    EXPECT_CALL(xWrapperMock, eventData)
        .WillOnce(
            testing::Return(testing::ByMove<EvgetX11::XEventPointer>({&xEvent.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(xWrapperMock, nextEvent)
        .WillOnce(testing::Return(testing::ByMove(EvgetX11TestUtils::createXEvent(device_event))));
    EXPECT_CALL(xWrapperMock, getDeviceButtonMapping)
        .WillOnce(
            testing::Return(testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1)))
        );

    EXPECT_CALL(xWrapperMock, atomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, &XFree})));
    EXPECT_CALL(xWrapperMock, getActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, getFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, query_pointer).WillRepeatedly([]() {
        return EvgetX11TestUtils::create_pointer_result();
    });
}

void EvgetX11TestUtils::set_x_wrapper_key_mocks(
    EvgetX11TestUtils::XWrapperMock& xWrapperMock,
    XIRawEvent& device_event,
    XEvent& xEvent
) {
    EXPECT_CALL(xWrapperMock, eventData)
        .WillOnce(
            testing::Return(testing::ByMove<EvgetX11::XEventPointer>({&xEvent.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(xWrapperMock, nextEvent)
        .WillOnce(testing::Return(testing::ByMove(EvgetX11TestUtils::createXEvent(device_event))));
    EXPECT_CALL(xWrapperMock, getActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, getFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(xWrapperMock, lookupCharacter)
        .WillOnce([](const XIRawEvent&, const EvgetX11::QueryPointerResult&, KeySym& keySym) {
            keySym = XK_A;
            return "a";
        });
    EXPECT_CALL(xWrapperMock, query_pointer).WillRepeatedly([]() {
        return EvgetX11TestUtils::create_pointer_result();
    });
}

void EvgetX11TestUtils::set_x_wrapper_mocks(EvgetX11TestUtils::XWrapperMock& xWrapperMock) {
    EXPECT_CALL(xWrapperMock, listInputDevices)
        .WillOnce(
            testing::Return(
                testing::ByMove<std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>>(
                    {nullptr, [](XDeviceInfo*) {}}
                )
            )
        );
    EXPECT_CALL(xWrapperMock, queryDevice)
        .WillOnce(
            testing::Return(
                testing::ByMove<std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>>(
                    {nullptr, [](XIDeviceInfo*) {}}
                )
            )
        );
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
