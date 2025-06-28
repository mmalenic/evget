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
//

#include "evgetcore/async/container/LockingVector.h"

#include <gtest/gtest.h>

#include <fmt/format.h>

TEST(LockingVectorTest, PushBackAndIntoInner) {
    auto vector = EvgetCore::LockingVector<int>{};
    vector.push_back(1);

    auto inner = *vector.into_inner();
    const std::vector expected{1};
    ASSERT_EQ(inner, std::vector{1});
}

TEST(LockingVectorTest, IntoInnerAt) {
    auto vector = EvgetCore::LockingVector<int>{};
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);

    ASSERT_FALSE(vector.into_inner_at(4).has_value());

    auto inner = vector.into_inner_at(2);
    const std::vector expected{1, 2, 3};
    ASSERT_EQ(inner, expected);
}

TEST(LockingVectorTest, UnsafePushBackAndIntoInner) {
    auto vector = EvgetCore::LockingVector<int>{};
    vector.unsafe_push_back(1);

    auto inner = *vector.unsafe_into_inner();
    const std::vector expected{1};
    ASSERT_EQ(inner, std::vector{1});
}

TEST(LockingVectorTest, UnsafeIntoInnerAt) {
    auto vector = EvgetCore::LockingVector<int>{};
    vector.unsafe_push_back(1);
    vector.unsafe_push_back(2);
    vector.unsafe_push_back(3);

    ASSERT_FALSE(vector.unsafe_into_inner_at(4).has_value());

    auto inner = vector.unsafe_into_inner_at(2);
    const std::vector expected{1, 2, 3};
    ASSERT_EQ(inner, expected);
}
