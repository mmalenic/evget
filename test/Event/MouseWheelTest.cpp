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
#include "evget/Event/MouseWheel.h"
#include "TestUtils/Event/EventTestUtils.h"

namespace EventTestUtils = TestUtils::EventTestUtils;

TEST(MouseWheelTest, Time) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::MouseWheel::MouseWheelBuilder{}.time(std::chrono::nanoseconds{1}).build(), 0, "1");
}

TEST(MouseWheelTest, Type) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::MouseWheel::MouseWheelBuilder{}.type(Event::Common::Device::Mouse).build(), 1, "Mouse");
}

TEST(MouseWheelTest, WheelDown) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::MouseWheel::MouseWheelBuilder{}.wheelDown(1).build(), 2, "1");
}

TEST(MouseWheelTest, WheelUp) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::MouseWheel::MouseWheelBuilder{}.wheelUp(1).build(), 3, "1");
}