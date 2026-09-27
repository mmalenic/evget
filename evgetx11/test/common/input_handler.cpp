#include "common/input_handler.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp> // NOLINT(misc-include-cleaner)
#include <boost/asio/detached.hpp>
#include <boost/asio/thread_pool.hpp>
#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <future>
#include <utility>

#include "evget/error.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"

test::Pipe::Pipe() {
    std::array<int, 2> ends{-1, -1};
    if (pipe2(ends.data(), O_CLOEXEC) == 0) {
        read_.reset(ends.front());
        write_.reset(ends.back());
    }
}

int test::Pipe::ReadEnd() const {
    return read_.get();
}

bool test::Pipe::Write() const {
    constexpr char kByte = 1;
    return write(write_.get(), &kByte, sizeof(kByte)) == sizeof(kByte);
}

// NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
boost::asio::awaitable<void>
test::RunNext(const evgetx11::InputHandler& handler, std::promise<evget::Result<evgetx11::InputEvent>> promise) {
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    promise.set_value(co_await handler.Next());
}

// NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)

std::future<evget::Result<evgetx11::InputEvent>>
test::SpawnNext(const evgetx11::InputHandler& handler, boost::asio::thread_pool& pool) {
    std::promise<evget::Result<evgetx11::InputEvent>> promise{};
    auto future = promise.get_future();
    // NOLINTNEXTLINE(misc-include-cleaner)
    boost::asio::co_spawn(pool, RunNext(handler, std::move(promise)), boost::asio::detached);
    return future;
}
