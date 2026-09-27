#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/extensions/XI2.h>
#include <boost/asio/thread_pool.hpp>

#include <array>
#include <chrono>
#include <future>

// clang-format off
#include "common/input_handler.h"
// clang-format on
#include "common/x11_mock.h"
#include "evgetx11/input_handler.h"

TEST(InputHandlerTest, RequestStopWaitingNext) {
    boost::asio::thread_pool pool{1};
    test::X11ApiMock x_wrapper_mock{};
    const test::Pipe connection{};
    ASSERT_GE(connection.ReadEnd(), 0);

    EXPECT_CALL(x_wrapper_mock, Pending).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(x_wrapper_mock, GetConnectionNumber).WillOnce(testing::Return(connection.ReadEnd()));
    EXPECT_CALL(x_wrapper_mock, NextEvent).Times(0);
    evgetx11::InputHandler handler{x_wrapper_mock};

    auto next = test::SpawnNext(handler, pool);
    ASSERT_EQ(next.wait_for(std::chrono::milliseconds{100}), std::future_status::timeout);

    handler.RequestStop();
    ASSERT_EQ(next.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_FALSE(next.get().has_value());
    pool.join();
}

TEST(InputHandlerTest, NextWaitsForConnection) {
    boost::asio::thread_pool pool{1};
    test::X11ApiMock x_wrapper_mock{};
    const test::Pipe connection{};
    ASSERT_GE(connection.ReadEnd(), 0);
    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto raw = test::CreateXiRawEvent(XI_RawKeyPress, valuator_mask, values);

    EXPECT_CALL(x_wrapper_mock, Pending).WillOnce(testing::Return(0)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(x_wrapper_mock, GetConnectionNumber).WillOnce(testing::Return(connection.ReadEnd()));
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(test::CreateXEvent(raw)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    const evgetx11::InputHandler handler{x_wrapper_mock};

    auto next = test::SpawnNext(handler, pool);
    ASSERT_EQ(next.wait_for(std::chrono::milliseconds{100}), std::future_status::timeout);

    ASSERT_TRUE(connection.Write());
    ASSERT_EQ(next.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    auto event = next.get();
    ASSERT_TRUE(event.has_value());
    ASSERT_EQ(event->GetEventType(), XI_RawKeyPress);
    pool.join();
}

TEST(InputHandlerTest, RequestStopEvents) {
    boost::asio::thread_pool pool{1};
    test::X11ApiMock x_wrapper_mock{};
    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto raw = test::CreateXiRawEvent(XI_RawKeyRelease, valuator_mask, values);

    EXPECT_CALL(x_wrapper_mock, Pending).WillOnce(testing::Return(1)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(x_wrapper_mock, GetConnectionNumber).Times(0);
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(test::CreateXEvent(raw)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    evgetx11::InputHandler handler{x_wrapper_mock};

    handler.RequestStop();

    auto first = test::SpawnNext(handler, pool);
    ASSERT_EQ(first.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    auto event = first.get();
    ASSERT_TRUE(event.has_value());
    ASSERT_EQ(event->GetEventType(), XI_RawKeyRelease);

    auto second = test::SpawnNext(handler, pool);
    ASSERT_EQ(second.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_FALSE(second.get().has_value());
    pool.join();
}
