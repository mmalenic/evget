#include "evgetwindows/message_window.h"

#include <gtest/gtest.h>

#include <boost/asio/co_spawn.hpp> // NOLINT(misc-include-cleaner)
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_future.hpp>
#include <windows.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "common/log.h"
#include "common/windows_mock.h"
#include "evgetwindows/raw_event.h"

namespace {
using evgetwindows::EnqueueOutcome;
using evgetwindows::MessageWindow;
using test::AsRawInput;
using test::CapturedLog;
using test::MakeDeviceChangeRawEvent;
using test::MakeHidPacket;
using test::MakeHidPacketDistinct;
using test::MakeHidPacketUndersized;
using test::MakeHidRawInput;
using test::MakeHidReportBytes;
using test::MakeKeyboardRawInput;
using test::MakeMouseRawInput;
} // namespace

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,bugprone-unchecked-optional-access,clang-analyzer-cplusplus.NewDelete)

TEST(MessageWindowTest, ToRawEventClassifiesMouse) {
    const auto event = MessageWindow::ToRawEvent(MakeMouseRawInput(0x11, 0x22), sizeof(RAWINPUT));

    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->header.dwType, static_cast<DWORD>(RIM_TYPEMOUSE));
    ASSERT_TRUE(std::holds_alternative<RAWMOUSE>(event->data));
    EXPECT_EQ(std::get<RAWMOUSE>(event->data).lLastX, 0x11);
    EXPECT_EQ(std::get<RAWMOUSE>(event->data).lLastY, 0x22);
}

TEST(MessageWindowTest, ToRawEventClassifiesKeyboard) {
    const auto event = MessageWindow::ToRawEvent(MakeKeyboardRawInput(0x41), sizeof(RAWINPUT));

    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->header.dwType, static_cast<DWORD>(RIM_TYPEKEYBOARD));
    ASSERT_TRUE(std::holds_alternative<RAWKEYBOARD>(event->data));
    EXPECT_EQ(std::get<RAWKEYBOARD>(event->data).VKey, 0x41);
}

TEST(MessageWindowTest, ToRawEventIgnoresOther) {
    EXPECT_FALSE(MessageWindow::ToRawEvent(MakeHidRawInput(), sizeof(RAWINPUT)).has_value());
}

TEST(MessageWindowTest, EnqueueSendsEvents) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    EXPECT_EQ(window.Enqueue(MakeMouseRawInput(1, 2), sizeof(RAWINPUT)), EnqueueOutcome::kSent);
    EXPECT_EQ(window.Enqueue(MakeKeyboardRawInput(0x41), sizeof(RAWINPUT)), EnqueueOutcome::kSent);
}

TEST(MessageWindowTest, EnqueueIgnoresUnknown) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    EXPECT_EQ(window.Enqueue(MakeHidRawInput(), sizeof(RAWINPUT)), EnqueueOutcome::kIgnored);
}

TEST(MessageWindowTest, EnqueueDropsWhenFull) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    const RAWINPUT mouse = MakeMouseRawInput(1, 2);
    std::size_t sent = 0;
    for (std::size_t i = 0; i < evgetwindows::kRawEventChannelCapacity; ++i) {
        if (window.Enqueue(mouse, sizeof(RAWINPUT)) == EnqueueOutcome::kSent) {
            ++sent;
        }
    }

    EXPECT_EQ(sent, evgetwindows::kRawEventChannelCapacity);
    EXPECT_EQ(window.Enqueue(mouse, sizeof(RAWINPUT)), EnqueueOutcome::kDropped);
}

TEST(MessageWindowTest, ToRawEventClassifiesHid) {
    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacket(report, 1);

    const auto event = MessageWindow::ToRawEvent(AsRawInput(packet), packet.size());

    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->header.dwType, static_cast<DWORD>(RIM_TYPEHID));
    ASSERT_TRUE(std::holds_alternative<evgetwindows::HidPayload>(event->data));

    const auto& payload = std::get<evgetwindows::HidPayload>(event->data);
    EXPECT_EQ(payload.size, report.size());
    EXPECT_TRUE(std::ranges::equal(report, std::span{payload.report}.first(payload.size)));
}

TEST(MessageWindowTest, ToRawEventReadsReport) {
    const auto packet = MakeHidPacketDistinct(8, 2);
    const auto& raw = AsRawInput(packet);

    const auto first = MessageWindow::ToRawEventAt(raw, 0, packet.size());
    const auto second = MessageWindow::ToRawEventAt(raw, 1, packet.size());

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    ASSERT_TRUE(std::holds_alternative<evgetwindows::HidPayload>(first->data));
    ASSERT_TRUE(std::holds_alternative<evgetwindows::HidPayload>(second->data));

    EXPECT_EQ(std::get<evgetwindows::HidPayload>(first->data).report.at(0), static_cast<std::byte>(0));
    EXPECT_EQ(std::get<evgetwindows::HidPayload>(second->data).report.at(0), static_cast<std::byte>(1));
    EXPECT_FALSE(MessageWindow::ToRawEventAt(raw, 2, packet.size()).has_value());
}

TEST(MessageWindowTest, EnqueueSendsEventPerReport) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacket(report, 3);
    const auto& raw = AsRawInput(packet);

    std::size_t accepted = 0;
    for (std::size_t attempt = 0; attempt < evgetwindows::kRawEventChannelCapacity; ++attempt) {
        if (window.Enqueue(raw, packet.size()) == EnqueueOutcome::kDropped) {
            break;
        }
        ++accepted;
    }

    // Only true if every report in the packet became its own channel send.
    EXPECT_EQ(accepted, evgetwindows::kRawEventChannelCapacity / 3);
}

TEST(MessageWindowTest, EnqueueIgnoresLargeReport) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    const auto report = MakeHidReportBytes(evgetwindows::kHidReportCapacity + 1);
    const auto packet = MakeHidPacket(report, 1);

    EXPECT_EQ(window.Enqueue(AsRawInput(packet), packet.size()), EnqueueOutcome::kIgnored);
}

TEST(MessageWindowTest, ToRawEventReportBound) {
    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacket(report, 2);
    const auto& raw = AsRawInput(packet);

    EXPECT_TRUE(MessageWindow::ToRawEventAt(raw, 1, packet.size()).has_value());
    EXPECT_FALSE(MessageWindow::ToRawEventAt(raw, 1, packet.size() - 1).has_value());
}

TEST(MessageWindowTest, EnqueueIgnoresInvalidPacket) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacketUndersized(report, 1);

    EXPECT_EQ(window.Enqueue(AsRawInput(packet), packet.size()), EnqueueOutcome::kIgnored);
}

TEST(MessageWindowTest, DeviceChangeRetryWhenFull) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};
    int backing = 0;
    HANDLE device = &backing;

    const RAWINPUT mouse = MakeMouseRawInput(1, 2);
    while (window.Enqueue(mouse, sizeof(RAWINPUT)) == EnqueueOutcome::kSent) {
    }

    window.EnqueueDeviceChange(GIDC_REMOVAL, device);
    EXPECT_EQ(window.PendingDeviceChanges(), 1);

    ASSERT_TRUE(window.Channel().try_receive([](const auto& /*error*/, const evgetwindows::RawEvent& /*event*/) {}));

    // The next enqueue is what gives the held change its retry, so it reaches the channel ahead of the input.
    static_cast<void>(window.Enqueue(mouse, sizeof(RAWINPUT)));
    EXPECT_EQ(window.PendingDeviceChanges(), 0);
}

TEST(MessageWindowTest, ToDeviceChangeEvent) {
    int backing = 0;
    HANDLE device = &backing;

    const auto arrival = MakeDeviceChangeRawEvent(device, true);
    ASSERT_TRUE(std::holds_alternative<evgetwindows::DeviceChange>(arrival.data));
    EXPECT_TRUE(std::get<evgetwindows::DeviceChange>(arrival.data).arrival);
    EXPECT_EQ(std::get<evgetwindows::DeviceChange>(arrival.data).device, device);
    EXPECT_EQ(arrival.header.hDevice, device);

    const auto removal = MakeDeviceChangeRawEvent(device, false);
    ASSERT_TRUE(std::holds_alternative<evgetwindows::DeviceChange>(removal.data));
    EXPECT_FALSE(std::get<evgetwindows::DeviceChange>(removal.data).arrival);
    EXPECT_EQ(removal.header.hDevice, device);
}

TEST(MessageWindowTest, CloseRunsStop) {
    boost::asio::thread_pool pool{1};
    int calls = 0;
    MessageWindow window{pool.get_executor(), [&calls] { ++calls; }};

    EXPECT_EQ(window.HandleStopMessage(WM_CLOSE, 0), std::optional<LRESULT>{0});
    EXPECT_EQ(calls, 1);
}

TEST(MessageWindowTest, QueryEndSession) {
    boost::asio::thread_pool pool{1};
    int calls = 0;
    MessageWindow window{pool.get_executor(), [&calls] { ++calls; }};

    EXPECT_EQ(window.HandleStopMessage(WM_QUERYENDSESSION, 0), std::optional<LRESULT>{TRUE});
    EXPECT_EQ(calls, 0);
}

TEST(MessageWindowTest, EndSessionRunsStop) {
    boost::asio::thread_pool pool{1};
    int calls = 0;
    MessageWindow window{pool.get_executor(), [&calls] { ++calls; }};

    EXPECT_EQ(window.HandleStopMessage(WM_ENDSESSION, TRUE), std::optional<LRESULT>{0});
    EXPECT_EQ(calls, 1);
}

TEST(MessageWindowTest, CancelledEndSession) {
    boost::asio::thread_pool pool{1};
    int calls = 0;
    MessageWindow window{pool.get_executor(), [&calls] { ++calls; }};

    EXPECT_EQ(window.HandleStopMessage(WM_ENDSESSION, FALSE), std::optional<LRESULT>{0});
    EXPECT_EQ(calls, 0);
}

TEST(MessageWindowTest, MessagesNotHandled) {
    boost::asio::thread_pool pool{1};
    int calls = 0;
    MessageWindow window{pool.get_executor(), [&calls] { ++calls; }};

    EXPECT_FALSE(window.HandleStopMessage(WM_PAINT, 0).has_value());
    EXPECT_EQ(calls, 0);
}

TEST(MessageWindowTest, CloseWithoutCallback) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    EXPECT_EQ(window.HandleStopMessage(WM_CLOSE, 0), std::optional<LRESULT>{0});
}

TEST(MessageWindowTest, RequestStop) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    const std::vector<LONG> queued{1, 2, 3};
    for (const LONG last_x : queued) {
        ASSERT_EQ(window.Enqueue(MakeMouseRawInput(last_x, 0), sizeof(RAWINPUT)), EnqueueOutcome::kSent);
    }

    window.RequestStop();
    EXPECT_FALSE(window.Channel().is_open());

    // Stops at the first empty receive or error, so a 4th event without an error would show up.
    std::vector<LONG> received{};
    bool ended = false;
    while (!ended &&
           window.Channel().try_receive([&received, &ended](const auto& code, const evgetwindows::RawEvent& event) {
               const auto* mouse = std::get_if<RAWMOUSE>(&event.data);
               ended = code.failed() || mouse == nullptr;
               if (!ended) {
                   received.push_back(mouse->lLastX);
               }
           })) {
    }

    EXPECT_EQ(received, queued);
}

TEST(MessageWindowTest, QueueAfterStopRequest) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    window.RequestStop();

    EXPECT_EQ(window.Enqueue(MakeMouseRawInput(1, 2), sizeof(RAWINPUT)), EnqueueOutcome::kClosed);
}

TEST(MessageWindowTest, RequestStopTwice) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    window.RequestStop();
    window.RequestStop();

    EXPECT_FALSE(window.Channel().is_open());
}

TEST(MessageWindowTest, CapacityWarningBacklog) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};
    const CapturedLog log{};
    const RAWINPUT mouse = MakeMouseRawInput(1, 2);
    const auto warnings = [&log] {
        return std::ranges::count_if(log.Lines(), [](const std::string& line) {
            return line.contains("buffered events");
        });
    };

    for (std::size_t i = 1; i < evgetwindows::kChannelNearCapacity; ++i) {
        ASSERT_EQ(window.Enqueue(mouse, sizeof(RAWINPUT)), EnqueueOutcome::kSent);
    }
    EXPECT_EQ(warnings(), 0);
    ASSERT_EQ(window.Enqueue(mouse, sizeof(RAWINPUT)), EnqueueOutcome::kSent);
    EXPECT_EQ(window.Buffered(), evgetwindows::kChannelNearCapacity);
    EXPECT_EQ(warnings(), 1);

    while (window.Buffered() >= evgetwindows::kChannelNearCapacity / 2) {
        // NOLINTNEXTLINE(misc-include-cleaner)
        const auto [error, event] = boost::asio::co_spawn(pool, window.Receive(), boost::asio::use_future).get();
        ASSERT_FALSE(error);
    }

    while (window.Buffered() < evgetwindows::kChannelNearCapacity) {
        ASSERT_EQ(window.Enqueue(mouse, sizeof(RAWINPUT)), EnqueueOutcome::kSent);
    }
    EXPECT_EQ(warnings(), 2);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,bugprone-unchecked-optional-access,clang-analyzer-cplusplus.NewDelete)
