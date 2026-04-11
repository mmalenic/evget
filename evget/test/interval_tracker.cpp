#include "evget/interval_tracker.h"

#include <gtest/gtest.h>

#include <chrono>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,bugprone-unchecked-optional-access)

TEST(IntervalTrackerTest, FirstCallReturnsNullopt) {
    evget::IntervalTracker tracker{};

    auto result = tracker.Interval(1000);

    ASSERT_FALSE(result.has_value());
}

TEST(IntervalTrackerTest, ConsecutiveCallsReturnInterval) {
    evget::IntervalTracker tracker{};

    tracker.Interval(100);

    auto first = tracker.Interval(300);
    ASSERT_TRUE(first.has_value());
    ASSERT_EQ(*first, std::chrono::microseconds{200});

    auto second = tracker.Interval(600);
    ASSERT_TRUE(second.has_value());
    ASSERT_EQ(*second, std::chrono::microseconds{300});

    auto third = tracker.Interval(700);
    ASSERT_TRUE(third.has_value());
    ASSERT_EQ(*third, std::chrono::microseconds{100});
}

TEST(IntervalTrackerTest, TimeBackwardsReturnsNullopt) {
    evget::IntervalTracker tracker{};

    tracker.Interval(1000);
    auto result = tracker.Interval(500);

    ASSERT_FALSE(result.has_value());

    // After backwards time, previous is reset to the backwards value.
    auto next = tracker.Interval(800);
    ASSERT_TRUE(next.has_value());
    ASSERT_EQ(*next, std::chrono::microseconds{300});
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,bugprone-unchecked-optional-access)
