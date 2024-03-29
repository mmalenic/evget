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
#include "evgetx11/XInputHandler.h"
#include "evgetx11/XWrapper.h"

TEST(EventTransformerX11Test, TestConstructor) {  // NOLINT(cert-err58-cpp)
    //    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    //    EvgetX11TestUtils::XEventSwitchMock xEventSwitchMock{};
    //
    //    auto xDeviceInfo = EvgetX11TestUtils::createXDeviceInfo();
    //    std::array<XIAnyClassInfo *, 3> info{};
    //    char name[] ="name";
    //    auto xiDeviceInfo = EvgetX11TestUtils::createXIDeviceInfo(info, name);
    //
    //    EXPECT_CALL(xWrapperMock,
    //    listInputDevices).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<XDeviceInfo[],
    //    decltype(&XFreeDeviceList)>>({&xDeviceInfo, [](XDeviceInfo *_){}}))); EXPECT_CALL(xWrapperMock,
    //    queryDevice).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<XIDeviceInfo[],
    //    decltype(&XIFreeDeviceInfo)>>({&xiDeviceInfo, [](XIDeviceInfo *_){}}))); EXPECT_CALL(xWrapperMock,
    //    atomName).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char *)
    //    XI_MOUSE, [](void *_){ return 0; }})));
    //
    //    EXPECT_CALL(xEventSwitchMock, refreshDevices(1, testing::_, testing::_, testing::_));
    //
    //    EvgetX11::EventTransformerX11 transformer{xWrapperMock, xEventSwitchMock};
}

TEST(EventTransformerX11Test, TestTransformEvent) {  // NOLINT(cert-err58-cpp)
    //    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    //    EvgetX11TestUtils::XEventSwitchMock xEventSwitchMock{};
    //    EvgetX11::XInputEvent event = EvgetX11::XInputEvent::nextEvent(xWrapperMock);
    //
    //    EXPECT_CALL(xEventSwitchMock, switchOnEvent).WillOnce([](const EvgetX11::XInputEvent &_event,
    //    std::chrono::nanoseconds _timestamp, EvgetX11::EventData &data) {
    //        data.push_back(EvgetCore::Event::Data{"Test"});
    //        return true;
    //    });
    //
    //    EvgetX11::EventTransformerX11 transformer{xWrapperMock, xEventSwitchMock};
    //    auto data = transformer.transformEvent(std::move(event));

    // ASSERT_EQ(data.at(0).begin()->getName(), "Test");
}