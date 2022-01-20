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
#include "evget/Event/Touch.h"
#include "TestUtils/Event/EventTestUtils.h"

namespace EventTestUtils = TestUtils::EventTestUtils;

TEST(TouchTest, Time) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.time(std::chrono::nanoseconds{1}).build(), 0, "1");
}

TEST(TouchTest, PositionX) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.positionX(1).build(), 1, "1");
}

TEST(TouchTest, PositionY) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.positionY(1).build(), 2, "1");
}

TEST(TouchTest, EllipseMajorInner) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.ellipseMajorInner(1).build(), 3, "1");
}

TEST(TouchTest, EllipseMinorInner) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.ellipseMinorInner(1).build(), 4, "1");
}

TEST(TouchTest, Orientation) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.orientation(1).build(), 5, "1");
}

TEST(TouchTest, PositionXOuter) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.positionXOuter(1).build(), 6, "1");
}

TEST(TouchTest, PositionYOuter) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.positionYOuter(1).build(), 7, "1");
}

TEST(TouchTest, EllipseMajorOuter) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.ellipseMajorOuter(1).build(), 8, "1");
}

TEST(TouchTest, EllipseMinorOuter) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.ellipseMinorOuter(1).build(), 9, "1");
}

TEST(TouchTest, Multitouch) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(Event::Touch::TouchBuilder{}.multitouch(1).build(), 10, "1");
}