#include "evgetwindows/message_window.h"

#include <gtest/gtest.h>

#include <boost/asio/thread_pool.hpp>

#include <cstddef>
#include <variant>

#include "common/windows_mock.h"
#include "evgetwindows/raw_event.h"

namespace {
using evgetwindows::EnqueueOutcome;
using evgetwindows::MessageWindow;
using test::MakeHidRawInput;
using test::MakeKeyboardRawInput;
using test::MakeMouseRawInput;
} // namespace

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,bugprone-unchecked-optional-access,clang-analyzer-cplusplus.NewDelete)

TEST(MessageWindowTest, ToRawEventClassifiesMouse) {
    const auto event = MessageWindow::ToRawEvent(MakeMouseRawInput(0x11, 0x22));

    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->header.dwType, static_cast<DWORD>(RIM_TYPEMOUSE));
    ASSERT_TRUE(std::holds_alternative<RAWMOUSE>(event->data));
    EXPECT_EQ(std::get<RAWMOUSE>(event->data).lLastX, 0x11);
    EXPECT_EQ(std::get<RAWMOUSE>(event->data).lLastY, 0x22);
}

TEST(MessageWindowTest, ToRawEventClassifiesKeyboard) {
    const auto event = MessageWindow::ToRawEvent(MakeKeyboardRawInput(0x41));

    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->header.dwType, static_cast<DWORD>(RIM_TYPEKEYBOARD));
    ASSERT_TRUE(std::holds_alternative<RAWKEYBOARD>(event->data));
    EXPECT_EQ(std::get<RAWKEYBOARD>(event->data).VKey, 0x41);
}

TEST(MessageWindowTest, ToRawEventIgnoresOther) {
    EXPECT_FALSE(MessageWindow::ToRawEvent(MakeHidRawInput()).has_value());
}

TEST(MessageWindowTest, EnqueueSendsEvents) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    EXPECT_EQ(window.Enqueue(MakeMouseRawInput(1, 2)), EnqueueOutcome::kSent);
    EXPECT_EQ(window.Enqueue(MakeKeyboardRawInput(0x41)), EnqueueOutcome::kSent);
}

TEST(MessageWindowTest, EnqueueIgnoresUnknown) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    EXPECT_EQ(window.Enqueue(MakeHidRawInput()), EnqueueOutcome::kIgnored);
}

TEST(MessageWindowTest, EnqueueDropsWhenFull) {
    boost::asio::thread_pool pool{1};
    MessageWindow window{pool.get_executor()};

    const RAWINPUT mouse = MakeMouseRawInput(1, 2);
    std::size_t sent = 0;
    for (std::size_t i = 0; i < evgetwindows::kRawEventChannelCapacity; ++i) {
        if (window.Enqueue(mouse) == EnqueueOutcome::kSent) {
            ++sent;
        }
    }

    EXPECT_EQ(sent, evgetwindows::kRawEventChannelCapacity);
    EXPECT_EQ(window.Enqueue(mouse), EnqueueOutcome::kDropped);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,bugprone-unchecked-optional-access,clang-analyzer-cplusplus.NewDelete)
