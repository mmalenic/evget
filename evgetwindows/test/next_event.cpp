#include "evgetwindows/next_event.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/thread_pool.hpp>

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>

#include "common/windows_mock.h"
#include "evget/async/scheduler/scheduler.h"
#include "evget/error.h"
#include "evget/input_event.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"

namespace {
// NOLINTBEGIN(misc-include-cleaner)
using RawEventChannel =
    boost::asio::experimental::concurrent_channel<void(boost::system::error_code, evgetwindows::RawEvent)>;
using NextResult = evget::Result<evget::InputEvent<evgetwindows::RawEvent>>;
using SeamResult = std::tuple<boost::system::error_code, evgetwindows::RawEvent>;

// NOLINTEND(misc-include-cleaner)

// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
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
        return [](evgetwindows::RawEvent event) -> boost::asio::awaitable<SeamResult> {
            co_return SeamResult{boost::system::error_code{}, event};
        }(injected);
    });

    evgetwindows::WindowsApi& api = mock;
    EXPECT_TRUE(api.Start().has_value());

    std::optional<SeamResult> received{};
    scheduler->Spawn<SeamResult>(api.ReceiveNext(), [&](auto value) { received = std::move(value); });
    scheduler->Join();

    api.Stop();

    ASSERT_TRUE(received.has_value());
    const auto& [error, raw_event] = *received;
    ASSERT_FALSE(error);
    EXPECT_EQ(std::get<RAWKEYBOARD>(raw_event.data).VKey, std::get<RAWKEYBOARD>(injected.data).VKey);
}

TEST(NextEventTest, EventCrossesChannel) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    test::WindowsApiMock mock;

    const auto injected = test::MakeMouseRawEvent();
    EXPECT_CALL(mock, ReceiveNext()).WillOnce([injected] {
        return [](evgetwindows::RawEvent event) -> boost::asio::awaitable<SeamResult> {
            co_return SeamResult{boost::system::error_code{}, event};
        }(injected);
    });

    const evgetwindows::NextEvent next_event{mock};
    std::optional<NextResult> received{};
    scheduler->Spawn(GetNext(next_event, received), [] {});
    scheduler->Join();

    ASSERT_TRUE(received.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_TRUE(received->has_value());
    EXPECT_EQ(std::get<RAWMOUSE>((*received)->ViewData().data).lLastX, std::get<RAWMOUSE>(injected.data).lLastX);
    EXPECT_EQ(std::get<RAWMOUSE>((*received)->ViewData().data).lLastY, std::get<RAWMOUSE>(injected.data).lLastY);
}

TEST(ChannelBackpressureTest, DropOnIncrement) {
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
    boost::asio::thread_pool channel_pool{1};
    constexpr std::size_t kCapacity = 2;
    RawEventChannel channel{channel_pool.get_executor(), kCapacity};

    // Pump only ever calls try_send; a full buffer drops, never blocks.
    constexpr std::size_t kAttempts = 5;
    std::size_t dropped = 0;
    for (std::size_t i = 0; i < kAttempts; ++i) {
        if (!channel.try_send(boost::system::error_code{}, test::MakeMouseRawEvent())) {
            ++dropped;
        }
    }

    EXPECT_EQ(dropped, kAttempts - kCapacity);
}

TEST(NextEventTest, ChannelCloses) {
    auto scheduler = std::make_shared<evget::Scheduler>();
    test::WindowsApiMock mock;

    EXPECT_CALL(mock, ReceiveNext()).WillOnce([] {
        return []() -> boost::asio::awaitable<SeamResult> {
            co_return SeamResult{boost::asio::error::operation_aborted, evgetwindows::RawEvent{}};
        }();
    });

    const evgetwindows::NextEvent next_event{mock};
    std::optional<NextResult> received{};
    scheduler->Spawn(GetNext(next_event, received), [] {});
    scheduler->Join();

    ASSERT_TRUE(received.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_FALSE(received->has_value());
}
