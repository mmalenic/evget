#include "evgetwindows/console_stop.h"

#include <gtest/gtest.h>

#include <windows.h>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
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

TEST(ConsoleStopTest, DestructionWaitsForStop) {
    constexpr std::chrono::milliseconds kDestroyGrace{50};
    std::promise<void> entered;
    std::promise<void> release;
    auto released = release.get_future();
    std::atomic<bool> finished{false};
    auto console_stop = std::make_unique<ConsoleStop>([&entered, &released, &finished] {
        entered.set_value();
        released.wait();
        finished.store(true);
    });

    std::jthread handler{[] { ConsoleStop::Handle(CTRL_BREAK_EVENT); }};
    entered.get_future().wait();

    std::atomic<bool> destroyed{false};
    bool finished_before_destroyed = false;
    std::jthread destroyer{[&console_stop, &destroyed, &finished, &finished_before_destroyed] {
        console_stop.reset();
        finished_before_destroyed = finished.load();
        destroyed.store(true);
    }};

    std::this_thread::sleep_for(kDestroyGrace);
    EXPECT_FALSE(destroyed.load());

    release.set_value();
    destroyer.join();
    handler.join();

    EXPECT_TRUE(finished_before_destroyed);
}
