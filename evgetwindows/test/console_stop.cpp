#include "evgetwindows/console_stop.h"

#include <gtest/gtest.h>

#include <windows.h>

#include <chrono>
#include <stdexcept>
#include <thread>

using evgetwindows::ConsoleStop;

TEST(ConsoleStopTest, CtrlCIsCrt) {
    int calls = 0;
    const ConsoleStop console_stop{[&calls] { ++calls; }};

    EXPECT_EQ(ConsoleStop::Handle(CTRL_C_EVENT), FALSE);
    EXPECT_EQ(calls, 0);
}

TEST(ConsoleStopTest, BreakRunsStop) {
    int calls = 0;
    const ConsoleStop console_stop{[&calls] { ++calls; }};

    EXPECT_EQ(ConsoleStop::Handle(CTRL_BREAK_EVENT), TRUE);
    EXPECT_EQ(calls, 1);
}

TEST(ConsoleStopTest, CloseReturnsAfterStop) {
    constexpr std::chrono::milliseconds kStopDuration{20};
    bool finished = false;
    const ConsoleStop console_stop{[&finished, kStopDuration] {
        std::this_thread::sleep_for(kStopDuration);
        finished = true;
    }};

    EXPECT_EQ(ConsoleStop::Handle(CTRL_CLOSE_EVENT), TRUE);
    EXPECT_TRUE(finished);
}

TEST(ConsoleStopTest, LogoffAndShutdown) {
    int calls = 0;
    const ConsoleStop console_stop{[&calls] { ++calls; }};

    EXPECT_EQ(ConsoleStop::Handle(CTRL_LOGOFF_EVENT), FALSE);
    EXPECT_EQ(ConsoleStop::Handle(CTRL_SHUTDOWN_EVENT), FALSE);
    EXPECT_EQ(calls, 0);
}

TEST(ConsoleStopTest, NoStopAfterDestructor) {
    int calls = 0;
    {
        const ConsoleStop console_stop{[&calls] { ++calls; }};
    }

    EXPECT_EQ(ConsoleStop::Handle(CTRL_BREAK_EVENT), FALSE);
    EXPECT_EQ(calls, 0);
}

TEST(ConsoleStopTest, ThrowingCallback) {
    const ConsoleStop console_stop{[] { throw std::runtime_error{"stop failed"}; }};

    BOOL result = FALSE;
    EXPECT_NO_THROW(result = ConsoleStop::Handle(CTRL_BREAK_EVENT));
    EXPECT_EQ(result, TRUE);
}
