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
#include "TestUtils/Event/EventTestUtils.h"
#include "evget/Event/Common/Type.h"

namespace EventTestUtils = TestUtils::EventTestUtils;

TEST(TypeTest, Constructor) { // NOLINT(cert-err58-cpp)
    EventTestUtils::fieldValueAndName<Event::Common::Type>("Type", "Type", "Type");
}

TEST(TypeTest, Mouse) { // NOLINT(cert-err58-cpp)
    auto type = Event::Common::Type::createType(Event::Common::Type::Device::Mouse);
    ASSERT_EQ("Type", type->getName());
    ASSERT_EQ("Mouse", type->getEntry());
}

TEST(TypeTest, Keyboard) { // NOLINT(cert-err58-cpp)
    auto type = Event::Common::Type::createType(Event::Common::Type::Device::Keyboard);
    ASSERT_EQ("Type", type->getName());
    ASSERT_EQ("Keyboard", type->getEntry());
}

TEST(TypeTest, Touchpad) { // NOLINT(cert-err58-cpp)
    auto type = Event::Common::Type::createType(Event::Common::Type::Device::Touchpad);
    ASSERT_EQ("Type", type->getName());
    ASSERT_EQ("Touchpad", type->getEntry());
}

TEST(TypeTest, Touchscreen) { // NOLINT(cert-err58-cpp)
    auto type = Event::Common::Type::createType(Event::Common::Type::Device::Touchscreen);
    ASSERT_EQ("Type", type->getName());
    ASSERT_EQ("Touchscreen", type->getEntry());
}