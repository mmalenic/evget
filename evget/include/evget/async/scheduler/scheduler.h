#ifndef EVGET_ASYNC_SCHEDULER_SCHEDULER_H
#define EVGET_ASYNC_SCHEDULER_SCHEDULER_H

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/thread_pool.hpp>
#include <spdlog/spdlog.h>

#include <atomic>
#include <cstddef>
#include <exception>
#include <thread>
#include <utility>

#include "evget/error.h"

namespace evget {

/**
 * \brief Check whether the template parameter is a handler with a stop function.
 */
template <typename T>
concept HandlerWithStop = requires(T task) {
    { task.Stop() }; // NOLINT(readability/braces)
};

/**
 * \brief A wrapper around `asio::thread_pool` that provides methods for spawning tasks.
 */
class Scheduler {
public:
    /**
     * \brief Create a default scheduler with `2 * std::thread::hardware_concurrency()` threads.
     */
    Scheduler() = default;

    /**
     * \brief Create a scheduler with `n_threads` threads.
     * \param n_threads number of threads
     */
    explicit Scheduler(std::size_t n_threads);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception.
     * \param task task awaitable
     */
    void Spawn(boost::asio::awaitable<void>&& task);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception.
     * \tparam T return type for the task
     * \param task task awaitable
     */
    template <typename T>
    void Spawn(boost::asio::awaitable<T>&& task);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception and stops the inner running task handler
     *        on a result error.
     * \param task task awaitable
     * \param stop stop handler
     * \param ret_code return code
     */
    template <HandlerWithStop S>
    void SpawnResult(boost::asio::awaitable<Result<void>>&& task, S& stop, int& ret_code);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception and stops the inner running task handler
     *        on a result error.
     * \tparam T return type for task
     * \param task task awaitable
     * \param stop stop handler
     * \param ret_code return code
     */
    template <typename T, HandlerWithStop S>
    void SpawnResult(boost::asio::awaitable<Result<T>>&& task, S& stop, int& ret_code);

    /**
     * \brief Spawn a task.
     * \param task task awaitable
     * \param handler handler on completion
     */
    void Spawn(boost::asio::awaitable<void>&& task, Invocable<void> auto&& handler);

    /**
     * \brief Spawn a task.
     * \tparam T return type for the task
     * \param task task awaitable
     * \param handler handler on completion
     */
    template <typename T>
    void Spawn(boost::asio::awaitable<T>&& task, Invocable<void, T> auto&& handler);

    /**
     * \brief Join the scheduler, awaiting all tasks to complete.
     */
    void Join();

    /**
     * \brief Stop all thread pool threads as soon as possible.
     */
    void Stop();

    /**
     * \brief Whether the scheduler has been stopped.
     * \return stopped value
     */
    [[nodiscard]] boost::asio::awaitable<bool> IsStopped() const;

private:
    boost::asio::thread_pool pool_{DefaultThreadPoolSize()};
    std::atomic<bool> stopped_{false};

    void SpawnImpl(boost::asio::awaitable<void>&& task, Invocable<void> auto&& handler, boost::asio::thread_pool& pool);
    template <typename T>
    void SpawnImpl(boost::asio::awaitable<T>&& task, Invocable<void, T> auto&& handler, boost::asio::thread_pool& pool);

    static std::size_t DefaultThreadPoolSize();
    void LogException(const std::exception_ptr& error);
};

inline std::size_t Scheduler::DefaultThreadPoolSize() {
    const auto num_threads = std::thread::hardware_concurrency() * 2;
    return num_threads == 0 ? 2 : num_threads;
}

template <typename T>
void Scheduler::SpawnImpl(
    boost::asio::awaitable<T>&& task,
    Invocable<void, T> auto&& handler,
    boost::asio::thread_pool& pool
) {
    boost::asio::co_spawn(pool, std::move(task), [this, handler](const std::exception_ptr& err, T value) {
        LogException(err);
        handler(value);
    });
}

template <typename T>
void Scheduler::Spawn(boost::asio::awaitable<T>&& task) {
    SpawnImpl<T>(std::move(task), [this](auto) {}, pool_);
}

template <HandlerWithStop S>
void Scheduler::SpawnResult(boost::asio::awaitable<Result<void>>&& task, S& stop, int& ret_code) {
    Scheduler::Spawn(std::move(task), [&ret_code, &stop](Result<void> err) {
        stop.Stop();
        if (!err.has_value()) {
            ret_code = 1;
            spdlog::error("{}", err.error());
        }
    });
}

template <typename T, HandlerWithStop S>
void Scheduler::SpawnResult(boost::asio::awaitable<Result<T>>&& task, S& stop, int& ret_code) {
    Scheduler::Spawn(std::move(task), [&ret_code, &stop](Result<T> err) {
        stop.Stop();
        if (!err.has_value()) {
            ret_code = 1;
            spdlog::error("{}", err.error());
        }
    });
}

template <typename T>
void Scheduler::Spawn(boost::asio::awaitable<T>&& task, Invocable<void, T> auto&& handler) {
    SpawnImpl<T>(std::move(task), std::forward<decltype(handler)>(handler), pool_);
}

void Scheduler::SpawnImpl(
    boost::asio::awaitable<void>&& task,
    Invocable<void> auto&& handler,
    boost::asio::thread_pool& pool
) {
    boost::asio::co_spawn(pool, std::move(task), [this, handler](const std::exception_ptr& err) {
        LogException(err);
        handler();
    });
}

void Scheduler::Spawn(boost::asio::awaitable<void>&& task, Invocable<void> auto&& handler) {
    SpawnImpl(std::move(task), std::forward<decltype(handler)>(handler), pool_);
}

} // namespace evget

#endif
