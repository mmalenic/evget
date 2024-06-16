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

#include <X11/Xlib.h>

#include <array>

#include "EvgetX11TestUtils.h"
#include "evgetx11/EventTransformerX11.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XEventSwitchPointerKey.h"
#include "evgetx11/XInputHandler.h"
#include "evgetx11/XWrapper.h"

TEST(EventTransformerX11Test, TestTransformEvent) {  // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock, {}};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock, xEventSwitch};

    EXPECT_CALL(xWrapperMock, listInputDevices)
    .WillOnce(testing::Return(
    testing::ByMove<std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>>({nullptr, [](XDeviceInfo* _) {
        }})
    ));
    EXPECT_CALL(xWrapperMock, queryDevice)
    .WillOnce(testing::Return(
    testing::ByMove<std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>>({nullptr, [](XIDeviceInfo* _) {
        }})
    ));
    auto transformer = EvgetX11::EventTransformerX11{xWrapperMock, xEventSwitchPointerKey};

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

    xEventSwitchPointerKey.refreshDevices(1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo);
    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    auto data = transformer.transformEvent(std::move(inputEvent));
    auto entries = data.entries();

    ASSERT_EQ(entries.at(0).type(), EvgetCore::Event::EntryType::MouseClick);
    ASSERT_EQ(entries.at(0).data().at(0), "");
    ASSERT_EQ(entries.at(0).data().at(2), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(3), EvgetCore::Event::fromDouble(1.0));
    ASSERT_EQ(entries.at(0).data().at(4), "name");
    ASSERT_EQ(entries.at(0).data().at(11), "0");
    ASSERT_EQ(entries.at(0).data().at(12), "0");
    ASSERT_EQ(entries.at(0).data().at(13), "MOUSE");
    ASSERT_EQ(entries.at(0).data().at(14), "0");
}