#ifndef EVGET_ASYNC_SCHEDULER_SCHEDULER_H
#define EVGET_ASYNC_SCHEDULER_SCHEDULER_H

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/thread_pool.hpp>

#include <atomic>
#include <cstddef>
#include <exception>
#include <thread>
#include <utility>

#include "evget/error.h"

namespace evget {

namespace asio = boost::asio;

/**
 * \brief A wrapper around `asio::thread_pool` that provides a `spawn` method for spawning tasks.
 */
class Scheduler {
public:
    /**
     * \brief Create a default scheduler with `2 * std::thread::hardware_concurrency()` threads.
     */
    Scheduler() = default;

    /**
     * \brief Create a scheduler with `nThreads` threads.
     * \param nThreads number of threads.
     */
    explicit Scheduler(std::size_t n_threads);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception.
     * \param task task awaitable.
     */
    void Spawn(asio::awaitable<void>&& task);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception.
     * \tparam T return type for task.
     * \param task task awaitable.
     */
    template <typename T>
    void Spawn(asio::awaitable<T>&& task);

    /**
     * \brief Spawn a task.
     * \param task task awaitable.
     * \param handler handler on completion.
     */
    void Spawn(asio::awaitable<void>&& task, Invocable<void> auto&& handler);

    /**
     * \brief Spawn a task.
     * \tparam T return type for task.
     * \param task task awaitable.
     * \param handler handler on completion.
     */
    template <typename T>
    void Spawn(asio::awaitable<T>&& task, Invocable<void, T> auto&& handler);

    /**
     * \brief Join the scheduler, awaiting for all tasks to complete.
     */
    void Join();

    /**
     * \brief Stop all thread pool threads as soon as possible.
     */
    void Stop();

    /**
     * \brief Whether the scheduler has been stopped.
     * \return stopped value.
     */
    [[nodiscard]] asio::awaitable<bool> IsStopped() const;

private:
    asio::thread_pool pool_{DefaultThreadPoolSize()};
    std::atomic<bool> stopped_{false};

    void SpawnImpl(asio::awaitable<void>&& task, Invocable<void> auto&& handler, asio::thread_pool& pool);
    template <typename T>
    void SpawnImpl(asio::awaitable<T>&& task, Invocable<void, T> auto&& handler, asio::thread_pool& pool);

    static std::size_t DefaultThreadPoolSize();
    void LogException(const std::exception_ptr& error);
};

inline std::size_t Scheduler::DefaultThreadPoolSize() {
    const auto num_threads = std::thread::hardware_concurrency() * 2;
    return num_threads == 0 ? 2 : num_threads;
}

template <typename T>
void Scheduler::SpawnImpl(asio::awaitable<T>&& task, Invocable<void, T> auto&& handler, asio::thread_pool& pool) {
    asio::co_spawn(pool, std::move(task), [this, handler](const std::exception_ptr& err, T value) {
        LogException(err);
        handler(value);
    });
}

template <typename T>
void Scheduler::Spawn(asio::awaitable<T>&& task) {
    SpawnImpl<T>(std::move(task), [this](auto) { this->Stop(); }, pool_);
}

template <typename T>
void Scheduler::Spawn(asio::awaitable<T>&& task, Invocable<void, T> auto&& handler) {
    SpawnImpl<T>(std::move(task), std::forward<decltype(handler)>(handler), pool_);
}

void Scheduler::SpawnImpl(asio::awaitable<void>&& task, Invocable<void> auto&& handler, asio::thread_pool& pool) {
    asio::co_spawn(pool, std::move(task), [this, handler](const std::exception_ptr& err) {
        LogException(err);
        handler();
    });
}

void Scheduler::Spawn(asio::awaitable<void>&& task, Invocable<void> auto&& handler) {
    SpawnImpl(std::move(task), std::forward<decltype(handler)>(handler), pool_);
}

}  // namespace evget

#endif
