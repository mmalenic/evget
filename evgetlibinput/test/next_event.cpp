#include "evgetlibinput/next_event.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/asio/thread_pool.hpp>
#include <boost/scope/unique_fd.hpp>
#include <libinput.h>
#include <sys/eventfd.h>

#include <chrono>
#include <future>

#include "common/next_event.h"
#include "common/pipe.h"
#include "common/test_helpers.h"
#include "evget/error.h"
#include "evgetlibinput/libinput.h"

TEST(LibInputNextEventTest, RequestStopNext) {
    boost::asio::thread_pool pool{1};
    test::LibInputApiMock libinput_mock{};
    const test::Pipe input{};
    ASSERT_GE(input.ReadEnd(), 0);

    EXPECT_CALL(libinput_mock, HasPendingEvent).WillRepeatedly(testing::Return(evget::Result<bool>{false}));
    EXPECT_CALL(libinput_mock, HasQueuedEvent).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(libinput_mock, GetFd).WillOnce(testing::Return(input.ReadEnd()));
    EXPECT_CALL(libinput_mock, GetEvent).Times(0);
    evgetlibinput::NextEvent next_event{libinput_mock, boost::scope::unique_fd{eventfd(0, EFD_CLOEXEC)}};

    auto next = test::SpawnNext(next_event, pool);
    ASSERT_EQ(next.wait_for(std::chrono::milliseconds{100}), std::future_status::timeout);

    next_event.RequestStop();
    ASSERT_EQ(next.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_FALSE(next.get().has_value());
    pool.join();
}

TEST(LibInputNextEventTest, NextWaitForInput) {
    boost::asio::thread_pool pool{1};
    test::LibInputApiMock libinput_mock{};
    const test::Pipe input{};
    ASSERT_GE(input.ReadEnd(), 0);

    EXPECT_CALL(libinput_mock, HasPendingEvent)
        .WillOnce(testing::Return(evget::Result<bool>{false}))
        .WillRepeatedly(testing::Return(evget::Result<bool>{true}));
    EXPECT_CALL(libinput_mock, GetFd).WillOnce(testing::Return(input.ReadEnd()));
    EXPECT_CALL(libinput_mock, GetEvent).WillOnce([] {
        return evget::Result<evgetlibinput::LibInputEvent>{{nullptr, libinput_event_destroy}};
    });
    const evgetlibinput::NextEvent next_event{libinput_mock, boost::scope::unique_fd{eventfd(0, EFD_CLOEXEC)}};

    auto next = test::SpawnNext(next_event, pool);
    ASSERT_EQ(next.wait_for(std::chrono::milliseconds{100}), std::future_status::timeout);

    ASSERT_TRUE(input.Write());
    ASSERT_EQ(next.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_TRUE(next.get().has_value());
    pool.join();
}

TEST(LibInputNextEventTest, RequestStopReturnsEvents) {
    boost::asio::thread_pool pool{1};
    test::LibInputApiMock libinput_mock{};

    EXPECT_CALL(libinput_mock, HasPendingEvent).Times(0);
    EXPECT_CALL(libinput_mock, HasQueuedEvent).WillOnce(testing::Return(true)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(libinput_mock, GetFd).Times(0);
    EXPECT_CALL(libinput_mock, GetEvent).WillOnce([] {
        return evget::Result<evgetlibinput::LibInputEvent>{{nullptr, libinput_event_destroy}};
    });
    evgetlibinput::NextEvent next_event{libinput_mock, boost::scope::unique_fd{eventfd(0, EFD_CLOEXEC)}};

    next_event.RequestStop();

    auto first = test::SpawnNext(next_event, pool);
    ASSERT_EQ(first.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_TRUE(first.get().has_value());

    auto second = test::SpawnNext(next_event, pool);
    ASSERT_EQ(second.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_FALSE(second.get().has_value());
    pool.join();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(LibInputNextEventTest, RequestStopNoNewInput) {
    boost::asio::thread_pool pool{1};
    test::LibInputApiMock libinput_mock{};

    EXPECT_CALL(libinput_mock, HasPendingEvent).WillOnce(testing::Return(evget::Result<bool>{true}));
    EXPECT_CALL(libinput_mock, HasQueuedEvent)
        .WillOnce(testing::Return(true))
        .WillOnce(testing::Return(true))
        .WillRepeatedly(testing::Return(false));
    EXPECT_CALL(libinput_mock, GetFd).Times(0);
    EXPECT_CALL(libinput_mock, GetEvent).Times(3).WillRepeatedly([] {
        return evget::Result<evgetlibinput::LibInputEvent>{{nullptr, libinput_event_destroy}};
    });
    evgetlibinput::NextEvent next_event{libinput_mock, boost::scope::unique_fd{eventfd(0, EFD_CLOEXEC)}};

    auto before_stop = test::SpawnNext(next_event, pool);
    ASSERT_EQ(before_stop.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_TRUE(before_stop.get().has_value());

    next_event.RequestStop();

    for (int i = 0; i < 2; i++) {
        auto queued = test::SpawnNext(next_event, pool);
        ASSERT_EQ(queued.wait_for(std::chrono::seconds{5}), std::future_status::ready);
        ASSERT_TRUE(queued.get().has_value());
    }
    auto drained = test::SpawnNext(next_event, pool);
    ASSERT_EQ(drained.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    ASSERT_FALSE(drained.get().has_value());
    pool.join();
}

TEST(LibInputNextEventTest, DispatchFailure) {
    boost::asio::thread_pool pool{1};
    test::LibInputApiMock libinput_mock{};

    EXPECT_CALL(libinput_mock, HasPendingEvent)
        .WillOnce(
            testing::Return(
                evget::Result<bool>{
                    evget::Err{{.error_type = evget::ErrorType::kEventHandlerError, .message = "dispatch"}}
                }
            )
        );
    EXPECT_CALL(libinput_mock, GetFd).Times(0);
    EXPECT_CALL(libinput_mock, GetEvent).Times(0);
    const evgetlibinput::NextEvent next_event{libinput_mock, boost::scope::unique_fd{eventfd(0, EFD_CLOEXEC)}};

    auto next = test::SpawnNext(next_event, pool);
    ASSERT_EQ(next.wait_for(std::chrono::seconds{5}), std::future_status::ready);
    auto result = next.get();
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().message, "dispatch");
    pool.join();
}
