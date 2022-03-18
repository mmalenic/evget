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

#include "listinputdevices/InputDevice.h"
#include "ListInputDevicesTestUtils.h"

#include <gtest/gtest.h>

TEST(EventDeviceTest, OrderWithAndWithoutSymlinks) { // NOLINT(cert-err58-cpp)
    ListInputDevices::InputDevice lt{
        "/dev/input/event3",
        "",
        "",
        "",
        ListInputDeviceTestUtils::createCapabilities()
    };
    ListInputDevices::InputDevice gt{
        "/dev/input/event0",
        {},
        {},
        "",
        ListInputDeviceTestUtils::createCapabilities()
    };
    ASSERT_LT(lt, gt);
}

TEST(EventDeviceTest, OrderBothWithSymlinks) { // NOLINT(cert-err58-cpp)
    ListInputDevices::InputDevice lt{
        "/dev/input/event3",
        "",
        "",
        "",
        ListInputDeviceTestUtils::createCapabilities()
    };
    ListInputDevices::InputDevice gt{
        "/dev/input/event4",
        "",
        "",
        "",
        ListInputDeviceTestUtils::createCapabilities()
    };
    ASSERT_LT(lt, gt);
}

TEST(EventDeviceTest, OrderBothWithoutSymlinks) { // NOLINT(cert-err58-cpp)
    ListInputDevices::InputDevice lt{
        "/dev/input/event3",
        {},
        {},
        "",
        ListInputDeviceTestUtils::createCapabilities()
    };
    ListInputDevices::InputDevice gt{
        "/dev/input/event4",
        {},
        {},
        "",
        ListInputDeviceTestUtils::createCapabilities()
    };
    ASSERT_LT(lt, gt);
}