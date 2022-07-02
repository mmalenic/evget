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
#include "evgetcore/Event/Key.h"
#include "EventTestUtils.h"

namespace EventTestUtils = TestUtils::EventTestUtils;

TEST(KeyTest, Time) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.time(std::chrono::nanoseconds{1}).build(), 0, "1");
}

TEST(MouseClickTest, Device) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.device(EvgetCore::Event::Common::Device::Mouse).build(), 1, "Mouse");
}

TEST(KeyTest, PositionX) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.positionX(1).build(), 2, "1");
}

TEST(KeyTest, PositionY) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.positionY(1).build(), 3, "1");
}

TEST(KeyTest, ButtonAction) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.action(EvgetCore::Event::Button::ButtonAction::Press).build(), 4, "Press");
}

TEST(KeyTest, Button) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.button(1).build(), 5, "1");
}

TEST(KeyTest, Name) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.name("1").build(), 6, "1");
}

TEST(KeyTest, Character) { // NOLINT(cert-err58-cpp)
    EventTestUtils::event_entry_at(EvgetCore::Event::Key::KeyBuilder{}.character("a").build(), 7, "a");
}