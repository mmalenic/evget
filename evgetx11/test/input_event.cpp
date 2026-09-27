#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include <array>
#include <utility>

#include "common/x11_mock.h"
// clang-format off
#include "evgetx11/input_event.h"
// clang-format on

TEST(InputEventTest, MovedEventKeepsData) {
    test::X11ApiMock x_wrapper_mock{};
    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto first_raw = test::CreateXiRawEvent(XI_RawKeyPress, valuator_mask, values);
    auto second_raw = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);

    EXPECT_CALL(x_wrapper_mock, NextEvent)
        .WillOnce(testing::Return(test::CreateXEvent(first_raw)))
        .WillOnce(testing::Return(test::CreateXEvent(second_raw)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);

    auto source = evgetx11::InputEvent::NextEvent(x_wrapper_mock);
    const auto moved = std::move(source);
    source = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    ASSERT_TRUE(moved.HasData());
    ASSERT_EQ(moved.GetEventType(), XI_RawKeyPress);
    ASSERT_EQ(&moved.ViewData<XIRawEvent>(), &first_raw);

    ASSERT_TRUE(source.HasData());
    ASSERT_EQ(source.GetEventType(), XI_RawButtonPress);
    ASSERT_EQ(&source.ViewData<XIRawEvent>(), &second_raw);
}
