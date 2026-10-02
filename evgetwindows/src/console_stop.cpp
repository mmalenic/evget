#include "evgetwindows/console_stop.h"

#include <spdlog/spdlog.h>

#include <exception>
#include <functional>
#include <mutex>
#include <utility>

namespace {

struct StopState {
    std::mutex mutex;
    std::function<void()> on_stop;
};

StopState& State() {
    static StopState state;
    return state;
}

} // namespace

evgetwindows::ConsoleStop::ConsoleStop(std::function<void()> on_stop) {
    {
        auto& state = State();
        const std::scoped_lock lock{state.mutex};
        state.on_stop = std::move(on_stop);
    }

    if (SetConsoleCtrlHandler(&ConsoleStop::Handle, TRUE) == FALSE) {
        spdlog::error("SetConsoleCtrlHandler failed: {}", GetLastError());
    }
}

evgetwindows::ConsoleStop::~ConsoleStop() {
    if (SetConsoleCtrlHandler(&ConsoleStop::Handle, FALSE) == FALSE) {
        spdlog::error("SetConsoleCtrlHandler failed: {}", GetLastError());
    }

    auto& state = State();
    const std::scoped_lock lock{state.mutex};
    state.on_stop = nullptr;
}

BOOL WINAPI evgetwindows::ConsoleStop::Handle(DWORD ctrl_type) {
    // Ctrl+C falls through to CRT, which sends SIGINT.
    if (ctrl_type != CTRL_BREAK_EVENT && ctrl_type != CTRL_CLOSE_EVENT) {
        return FALSE;
    }

    try {
        auto& state = State();
        // Held across the call so the destructor waits for a stop.
        const std::scoped_lock lock{state.mutex};
        if (!state.on_stop) {
            return FALSE;
        }

        state.on_stop();
    } catch (const std::exception& e) {
        spdlog::error("console stop failed: {}", e.what());
    } catch (...) {
        spdlog::error("console stop failed with an unknown error");
    }

    return TRUE;
}
