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

#include "EventTestUtils.h"
#include "evgetcore/Event/MouseScroll.h"

namespace EventTestUtils = TestUtils::EventTestUtils;

TEST(MouseScrollTest, Time) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(
        EvgetCore::Event::MouseScroll{}.interval(EvgetCore::Event::SchemaField::Interval{1}).build(),
        0,
        "1"
    );
}

TEST(MouseScrollTest, Device) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(
        EvgetCore::Event::MouseScroll{}.device(EvgetCore::Event::Device::Mouse).build(),
        1,
        "Mouse"
    );
}

TEST(MouseScrollTest, PositionX) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::MouseScroll{}.positionX(1).build(), 2, "1");
}

TEST(MouseScrollTest, PositionY) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::MouseScroll{}.positionY(1).build(), 3, "1");
}

TEST(MouseScrollTest, Up) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::MouseScroll{}.up(1).build(), 4, "1");
}

TEST(MouseScrollTest, Down) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::MouseScroll{}.down(1).build(), 5, "1");
}

TEST(MouseScrollTest, Left) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::MouseScroll{}.left(1).build(), 6, "1");
}

TEST(MouseScrollTest, Right) {  // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::MouseScroll{}.right(1).build(), 7, "1");
}