#include "evgetwindows/elevation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::HasSubstr;

TEST(ElevationTest, QueryReturnsValue) {
    EXPECT_TRUE(evgetwindows::IsElevated().has_value());
}

TEST(ElevationTest, ElevatedMessage) {
    EXPECT_THAT(evgetwindows::ElevationMessage(true), HasSubstr("running elevated"));
}

TEST(ElevationTest, UnelevatedMessage) {
    const auto message = evgetwindows::ElevationMessage(false);
    EXPECT_THAT(message, HasSubstr("not captured"));
    EXPECT_THAT(message, HasSubstr("administrator"));
}

TEST(ElevationTest, DifferenceMessages) {
    EXPECT_NE(evgetwindows::ElevationMessage(true), evgetwindows::ElevationMessage(false));
}
