// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <boost/asio.hpp>
#include <evgetcore/Error.h>
#include <spdlog/spdlog.h>

#include <cstddef>

namespace EvgetCore {

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
    explicit Scheduler(std::size_t nThreads);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception.
     * \param task task awaitable.
     */
    void spawn(Invocable<asio::awaitable<void>> auto&& task);

    /**
     * \brief Spawn a task. Stops the thread pool on an exception.
     * \tparam T return type for task.
     * \param task task awaitable.
     */
    template <typename T>
    void spawn(Invocable<asio::awaitable<T>> auto&& task);

    /**
     * \brief Spawn a task.
     * \param task task awaitable.
     * \param handler handler on completion.
     */
    void spawn(Invocable<asio::awaitable<void>> auto&& task, Invocable<void> auto&& handler);

    /**
     * \brief Spawn a task.
     * \tparam T return type for task.
     * \param task task awaitable.
     * \param handler handler on completion.
     */
    template <typename T>
    void spawn(Invocable<asio::awaitable<T>> auto&& task, Invocable<void, T> auto&& handler);

    /**
     * \brief Join the scheduler, awaiting for all tasks to complete.
     */
    void join();

    /**
     * \brief Stop all thread pool threads as soon as possible.
     */
    void stop();

    /**
     * \brief Whether the scheduler has been stopped.
     * \return stopped value.
     */
    asio::awaitable<bool> isStopped() const;

    /**
     * \brief Get the executor for the thread pool.
     * \return executor.
     */
    asio::thread_pool::executor_type getExecutor();

private:
    asio::thread_pool pool{default_thread_pool_size()};
    std::atomic<bool> stopped{false};

    void
    spawnImpl(Invocable<asio::awaitable<void>> auto&& task, Invocable<void> auto&& handler, asio::thread_pool& pool);
    template <typename T>
    void
    spawnImpl(Invocable<asio::awaitable<T>> auto&& task, Invocable<void, T> auto&& handler, asio::thread_pool& pool);

    static constexpr std::size_t default_thread_pool_size();
    void log_exception(const std::exception_ptr& error);
};

constexpr std::size_t Scheduler::default_thread_pool_size() {
    const auto num_threads = std::thread::hardware_concurrency() * 2;
    return num_threads == 0 ? 2 : num_threads;
}

template <typename T>
void Scheduler::spawnImpl(
    Invocable<asio::awaitable<T>> auto&& task,
    Invocable<void, T> auto&& handler,
    asio::thread_pool& pool
) {
    asio::co_spawn(
        pool,
        [this, task]() { return task(); },
        [this, handler](const std::exception_ptr& err, T value) {
            log_exception(err);
            handler(value);
        }
    );
}

template <typename T>
void Scheduler::spawn(Invocable<asio::awaitable<T>> auto&& task) {
    spawnImpl<T>(std::forward<decltype(task)>(task), [this](auto) { this->stop(); }, pool);
}

template <typename T>
void Scheduler::spawn(Invocable<asio::awaitable<T>> auto&& task, Invocable<void, T> auto&& handler) {
    spawnImpl<T>(std::forward<decltype(task)>(task), std::forward<decltype(handler)>(handler), pool);
}

void Scheduler::spawnImpl(
    Invocable<asio::awaitable<void>> auto&& task,
    Invocable<void> auto&& handler,
    asio::thread_pool& pool
) {
    asio::co_spawn(
        pool,
        [task]() { return task(); },
        [this, handler](const std::exception_ptr& err) {
            log_exception(err);
            handler();
        }
    );
}

void Scheduler::spawn(Invocable<asio::awaitable<void>> auto&& task) {
    spawnImpl(std::forward<decltype(task)>(task), [this] { this->stop(); }, pool);
}

void Scheduler::spawn(Invocable<asio::awaitable<void>> auto&& task, Invocable<void> auto&& handler) {
    spawnImpl(std::forward<decltype(task)>(task), std::forward<decltype(handler)>(handler), pool);
}

}  // namespace EvgetCore

#endif  // SCHEDULER_H
