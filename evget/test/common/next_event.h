#ifndef EVGET_TEST_COMMON_NEXT_EVENT_H
#define EVGET_TEST_COMMON_NEXT_EVENT_H

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp> // NOLINT(misc-include-cleaner)
#include <boost/asio/detached.hpp>
#include <boost/asio/thread_pool.hpp>

#include <future>
#include <utility>

#include "evget/error.h"
#include "evget/next_event.h"

namespace test {

/**
 * \brief Get the next event from a source.
 * \tparam T event type
 * \param source the source
 * \param promise set to the result of `Next`
 * \return awaitable that completes once the promise is set
 */
template <typename T>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
boost::asio::awaitable<void> RunNext(const evget::NextEvent<T>& source, std::promise<evget::Result<T>> promise);

/**
 * \brief Start `Next` of a source.
 * \tparam T event type
 * \param source the source
 * \param pool the pool
 * \return the result of `Next`
 */
template <typename T>
std::future<evget::Result<T>> SpawnNext(const evget::NextEvent<T>& source, boost::asio::thread_pool& pool);

template <typename T>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
boost::asio::awaitable<void> RunNext(const evget::NextEvent<T>& source, std::promise<evget::Result<T>> promise) {
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    promise.set_value(co_await source.Next());
}

template <typename T>
std::future<evget::Result<T>> SpawnNext(const evget::NextEvent<T>& source, boost::asio::thread_pool& pool) {
    std::promise<evget::Result<T>> promise{};
    auto future = promise.get_future();
    // NOLINTNEXTLINE(misc-include-cleaner)
    boost::asio::co_spawn(pool, RunNext(source, std::move(promise)), boost::asio::detached);
    return future;
}

} // namespace test

#endif
