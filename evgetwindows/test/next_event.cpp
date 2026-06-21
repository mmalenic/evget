#include <gtest/gtest.h>

#include <gmock/gmock.h>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/system/error_code.hpp>

#include <cstddef>
#include <memory>
#include <optional>

#include "common/windows_mock.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"
#include "evget/input_event.h"
#include "evgetwindows/next_event.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"

namespace {
using RawEventChannel =
    boost::asio::experimental::concurrent_channel<void(boost::system::error_code, evgetwindows::RawEvent)>;
using NextResult = evget::Result<evget::InputEvent<evgetwindows::RawEvent>>;

boost::asio::awaitable<void> GetNext(const evgetwindows::NextEvent& next_event, std::optional<NextResult>& out) {
    out = co_await next_event.Next();
}
} // namespace

TEST(WindowsMockTest, MockableApi) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    test::WindowsApiMock mock;

    const auto injected = test::MakeKeyboardRawEvent();
    EXPECT_CALL(mock, Start()).WillOnce(testing::Return(evget::Result<void>{}));
    EXPECT_CALL(mock, Stop()).Times(1);
    EXPECT_CALL(mock, ReceiveNext()).WillOnce([injected] {
        return [](evgetwindows::RawEvent event) -> boost::asio::awaitable<evget::Result<evgetwindows::RawEvent>> {
            co_return event;
        }(injected);
    });

    evgetwindows::WindowsApi& api = mock;
    EXPECT_TRUE(api.Start().has_value());

    std::optional<evget::Result<evgetwindows::RawEvent>> received{};
    scheduler->Spawn<evget::Result<evgetwindows::RawEvent>>(api.ReceiveNext(), [&](auto value) {
        received = std::move(value);
    });
    scheduler->Join();

    api.Stop();

    ASSERT_TRUE(received.has_value());
    ASSERT_TRUE(received->has_value());
    EXPECT_EQ((*received)->data.keyboard.VKey, injected.data.keyboard.VKey);
}

TEST(NextEventTest, EventCrossesChannel) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    test::WindowsApiMock mock;

    const auto injected = test::MakeMouseRawEvent();
    EXPECT_CALL(mock, ReceiveNext()).WillOnce([injected] {
        return [](evgetwindows::RawEvent event) -> boost::asio::awaitable<evget::Result<evgetwindows::RawEvent>> {
            co_return event;
        }(injected);
    });

    const evgetwindows::NextEvent next_event{mock};
    std::optional<NextResult> received{};
    scheduler->Spawn(GetNext(next_event, received), [] {});
    scheduler->Join();

    ASSERT_TRUE(received.has_value());
    ASSERT_TRUE(received->has_value());
    EXPECT_EQ((*received)->ViewData().data.mouse.lLastX, injected.data.mouse.lLastX);
    EXPECT_EQ((*received)->ViewData().data.mouse.lLastY, injected.data.mouse.lLastY);
}

TEST(ChannelBackpressureTest, DropOnIncrement) {
    boost::asio::thread_pool channel_pool{1};
    constexpr std::size_t capacity = 2;
    RawEventChannel channel{channel_pool.get_executor(), capacity};

    // Pump only ever calls try_send; a full buffer drops, never blocks.
    constexpr std::size_t attempts = 5;
    std::size_t dropped = 0;
    for (std::size_t i = 0; i < attempts; ++i) {
        if (!channel.try_send(boost::system::error_code{}, test::MakeMouseRawEvent())) {
            ++dropped;
        }
    }

    EXPECT_EQ(dropped, attempts - capacity);
}

TEST(NextEventTest, ChannelCloses) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    test::WindowsApiMock mock;

    EXPECT_CALL(mock, ReceiveNext()).WillOnce([] {
        return []() -> boost::asio::awaitable<evget::Result<evgetwindows::RawEvent>> {
            co_return evget::Err{{.error_type = evget::ErrorType::kAsyncError, .message = "channel closed"}};
        }();
    });

    const evgetwindows::NextEvent next_event{mock};
    std::optional<NextResult> received{};
    scheduler->Spawn(GetNext(next_event, received), [] {});
    scheduler->Join();

    ASSERT_TRUE(received.has_value());
    EXPECT_FALSE(received->has_value());
}
