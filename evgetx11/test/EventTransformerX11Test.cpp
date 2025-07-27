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

#include <X11/X.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include <array>
#include <utility>

#include "evgetcore/Event/Device.h"
#include "evgetcore/Event/Entry.h"
#include "evgetcore/Event/Schema.h"
#include "evgetx11/EventTransformerX11.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XEventSwitchPointerKey.h"
#include "evgetx11/XInputEvent.h"
#include "utils/EvgetX11TestUtils.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(EventTransformerX11Test, TestTransformEvent) {
    EvgetX11TestUtils::XWrapperMock xWrapperMock{};
    EvgetX11::XEventSwitch xEventSwitch{xWrapperMock};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointerKey{xWrapperMock};

    EvgetX11TestUtils::set_x_wrapper_mocks(xWrapperMock);
    auto transformer = EvgetX11::EventTransformerX11{xWrapperMock, xEventSwitch, xEventSwitchPointerKey};

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
    auto deviceEvent = EvgetX11TestUtils::createXIRawEvent(XI_RawButtonPress, valuatorMask, values);

    auto xEvent = EvgetX11TestUtils::createXEvent(deviceEvent);
    set_x_wrapper_event_mocks(xWrapperMock, deviceEvent, xEvent);

    xEventSwitchPointerKey.refreshDevices(1, 1, EvgetCore::Event::Device::Mouse, "name", xiDeviceInfo, xEventSwitch);
    auto inputEvent = EvgetX11::XInputEvent::nextEvent(xWrapperMock);

    auto data = transformer.transformEvent(std::move(inputEvent));
    const auto& entries = data.entries();

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

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
