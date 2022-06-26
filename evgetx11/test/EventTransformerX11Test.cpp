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
#include <X11/extensions/XTest.h>
#include "evgetx11/XInputHandler.h"
#include "evgetx11/XWrapper.h"
#include "evgetx11/EventTransformerX11.h"
#include <X11/Xlib.h>
#include "EvgetX11TestUtils.h"

TEST(EventTransformerX11Test, TestConstructor) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11TestUtils::XEventSwitchMock xEventSwitchMock{};

    XDeviceInfo deviceInfo = {
            .id = 0,
            .type = 0,
            .name = nullptr,
            .num_classes = 0,
            .use = IsXExtensionPointer,
            .inputclassinfo = nullptr
    };
    XIDeviceInfo xi2DeviceInfo{
            .deviceid = 0,
            .name = nullptr,
            .use = 0,
            .attachment = 0,
            .enabled = true,
            .num_classes = 0,
            .classes = nullptr
    };

    EXPECT_CALL(xWrapperMock, listInputDevices).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>>({&deviceInfo, [](XDeviceInfo *_){}})));
    EXPECT_CALL(xWrapperMock, queryDevice).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>>({&xi2DeviceInfo, [](XIDeviceInfo *_){}})));
    EXPECT_CALL(xWrapperMock, atomName).WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({(char *) XI_MOUSE, [](void *_){ return 0; }})));

    EXPECT_CALL(xEventSwitchMock, refreshDevices(0, testing::_, testing::_, testing::_));

    EvgetX11::EventTransformerX11 transformer{xWrapperMock, {xEventSwitchMock}};
}

TEST(EventTransformerX11Test, TestTransformEvent) { // NOLINT(cert-err58-cpp)
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11TestUtils::XEventSwitchMock xEventSwitchMock{};
    EvgetX11::XInputEvent event = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    EXPECT_CALL(xEventSwitchMock, switchOnEvent).WillOnce([](const EvgetX11::XInputEvent &_event, std::chrono::nanoseconds _timestamp, EvgetX11::XEventSwitch::EventData &data) {
        std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};
        std::unique_ptr<EvgetCore::Event::AbstractData> genericData = std::make_unique<EvgetCore::Event::Data>("Test", std::move(fields));
        data.push_back(EvgetCore::Event::TableData::TableDataBuilder{}.genericData(std::move(genericData)).build());
        return true;
    });

    EvgetX11::EventTransformerX11 transformer{xWrapperMock, {xEventSwitchMock}};
    auto data = transformer.transformEvent(std::move(event));

    ASSERT_EQ(data.at(0)->getGenericData()->getName(), "Test");
    ASSERT_EQ(data.at(0)->getSystemData(), nullptr);
}