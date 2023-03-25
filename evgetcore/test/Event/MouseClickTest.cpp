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

// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>

#include "EventTestUtils.h"
#include "evgetcore/Event/MouseClick.h"

// namespace EventTestUtils = TestUtils::EventTestUtils;
//
// TEST(MouseClickTest, Time) {  // NOLINT(cert-err58-cpp)
//     EventTestUtils::event_entry_at(
//         EvgetCore::Event::MouseClick{}.interval(EvgetCore::Event::SchemaField::Interval{1}).build(),
//         0,
//         "1"
//     );
// }
//
// TEST(MouseClickTest, Device) {  // NOLINT(cert-err58-cpp)
//     EventTestUtils::event_entry_at(
//         EvgetCore::Event::MouseClick{}.device(EvgetCore::Event::Device::Mouse).build(),
//         1,
//         "Mouse"
//     );
// }
//
// TEST(MouseClickTest, PositionX) {  // NOLINT(cert-err58-cpp)
//     EventTestUtils::event_entry_at(EvgetCore::Event::MouseClick{}.positionX(1).build(), 2, "1");
// }
//
// TEST(MouseClickTest, PositionY) {  // NOLINT(cert-err58-cpp)
//     EventTestUtils::event_entry_at(EvgetCore::Event::MouseClick{}.positionY(1).build(), 3, "1");
// }
//
// TEST(MouseClickTest, ButtonAction) {  // NOLINT(cert-err58-cpp)
//     EventTestUtils::event_entry_at(
//         EvgetCore::Event::MouseClick{}.action(EvgetCore::Event::ButtonAction::Press).build(),
//         4,
//         "Press"
//     );
// }
//
// TEST(MouseClickTest, Button) {  // NOLINT(cert-err58-cpp)
//     EventTestUtils::event_entry_at(EvgetCore::Event::MouseClick{}.button(1).build(), 5, "1");
// }
//
// TEST(MouseClickTest, Name) {  // NOLINT(cert-err58-cpp)
//     EventTestUtils::event_entry_at(EvgetCore::Event::MouseClick{}.name("name").build(), 6, "name");
// }