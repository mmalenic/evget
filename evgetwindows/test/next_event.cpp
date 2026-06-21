#include <gtest/gtest.h>

#include <gmock/gmock.h>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>

#include "common/windows_mock.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"

namespace {
using RawEventChannel =
    boost::asio::experimental::concurrent_channel<void(boost::system::error_code, evgetwindows::RawEvent)>;
using ReceiveResult = std::tuple<boost::system::error_code, evgetwindows::RawEvent>;

constexpr std::size_t kRawEventChannelCapacity = 8192;

boost::asio::awaitable<ReceiveResult> ReceiveFrom(RawEventChannel& channel) {
    co_return co_await channel.async_receive(boost::asio::as_tuple(boost::asio::use_awaitable));
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
    boost::asio::thread_pool channel_pool{1};
    RawEventChannel channel{channel_pool.get_executor(), kRawEventChannelCapacity};

    const auto injected = test::MakeMouseRawEvent();
    ASSERT_TRUE(channel.try_send(boost::system::error_code{}, injected));

    std::optional<ReceiveResult> received{};
    scheduler->Spawn<ReceiveResult>(ReceiveFrom(channel), [&](auto value) { received = std::move(value); });
    scheduler->Join();

    ASSERT_TRUE(received.has_value());
    const auto& [error_code, raw] = *received;
    EXPECT_FALSE(error_code);
    EXPECT_EQ(raw.data.mouse.lLastX, injected.data.mouse.lLastX);
    EXPECT_EQ(raw.data.mouse.lLastY, injected.data.mouse.lLastY);
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
    boost::asio::thread_pool channel_pool{1};
    RawEventChannel channel{channel_pool.get_executor(), kRawEventChannelCapacity};

    std::optional<ReceiveResult> received{};
    scheduler->Spawn<ReceiveResult>(ReceiveFrom(channel), [&](auto value) { received = std::move(value); });

    channel.close();
    scheduler->Join();

    ASSERT_TRUE(received.has_value());
    const auto& [error_code, raw] = *received;
    static_cast<void>(raw);
    EXPECT_TRUE(error_code);
}
