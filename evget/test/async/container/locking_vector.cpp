#include <gtest/gtest.h>

#include <vector>

// clang-format off
#include "evget/async/container/locking_vector.h"
// clang-format on

TEST(LockingVectorTest, PushBackAndIntoInner) {
    auto vector = evget::LockingVector<int>{};
    vector.PushBack(1);

    auto inner = vector.IntoInner();
    const std::vector expected{1};
    ASSERT_EQ(inner, std::vector{1});
}

TEST(LockingVectorTest, IntoInnerAt) {
    auto vector = evget::LockingVector<int>{};
    vector.PushBack(1);
    vector.PushBack(2);
    vector.PushBack(3);

    ASSERT_FALSE(vector.IntoInnerAt(4).has_value());

    auto inner = vector.IntoInnerAt(2);
    const std::vector expected{1, 2, 3};
    ASSERT_EQ(inner, expected);
}

TEST(LockingVectorTest, UnsafePushBackAndIntoInner) {
    auto vector = evget::LockingVector<int>{};
    vector.UnsafePushBack(1);

    auto inner = vector.UnsafeIntoInner();
    const std::vector expected{1};
    ASSERT_EQ(inner, std::vector{1});
}

TEST(LockingVectorTest, UnsafeIntoInnerAt) {
    auto vector = evget::LockingVector<int>{};
    vector.UnsafePushBack(1);
    vector.UnsafePushBack(2);
    vector.UnsafePushBack(3);

    ASSERT_FALSE(vector.UnsafeIntoInnerAt(4).has_value());

    auto inner = vector.UnsafeIntoInnerAt(2);
    const std::vector expected{1, 2, 3};
    ASSERT_EQ(inner, expected);
}
