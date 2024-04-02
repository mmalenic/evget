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
#include "util/Util.h"
#include <spdlog/spdlog.h>

namespace Async {

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
     * \brief Spawn a task.
     * \param task task awaitable.
     * \param handler handler on completion.
     */
    void spawn(Util::Invocable<asio::awaitable<void>> auto&& task, Util::Invocable<void> auto&& handler);

    /**
     * \brief Spawn a task.
     * \tparam T return type for task.
     * \param task task awaitable.
     * \param handler handler on completion.
     */
    template <typename T>
    void spawn(Util::Invocable<asio::awaitable<T>> auto&& task, Util::Invocable<void, T> auto&& handler);

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
    asio::awaitable<bool> isStopped();

    /**
     * \brief Get the executor for the thread pool.
     * \return executor.
     */
    asio::thread_pool::executor_type getExecutor();

private:
    asio::thread_pool pool{default_thread_pool_size()};
    std::atomic<bool> stopped{false};

    void spawnImpl(Util::Invocable<asio::awaitable<void>> auto&& task, Util::Invocable<void> auto&& handler, asio::thread_pool& pool);
    template <typename T>
    void spawnImpl(Util::Invocable<asio::awaitable<T>> auto&& task, Util::Invocable<void, T> auto&& handler, asio::thread_pool& pool);

    static constexpr std::size_t default_thread_pool_size();
    void log_exception(std::exception_ptr e);
};

constexpr std::size_t Scheduler::default_thread_pool_size() {
    std::size_t num_threads = std::thread::hardware_concurrency() * 2;
    return num_threads == 0 ? 2 : num_threads;
}

template <typename T>
void Scheduler::spawnImpl(
    Util::Invocable<asio::awaitable<T>> auto&& task,
    Util::Invocable<void, T> auto&& handler,
    asio::thread_pool& pool
) {
    asio::co_spawn(pool, [this, task]() {
        return task();
    }, [this, handler](std::exception_ptr e, T value) {
        log_exception(e);
        handler(value);
    });
}

template <typename T>
void Scheduler::spawn(
    Util::Invocable<asio::awaitable<T>> auto&& task,
    Util::Invocable<void, T> auto&& handler
) {
    spawnImpl<T>(std::forward<decltype(task)>(task), std::forward<decltype(handler)>(handler), pool);
}

void Scheduler::spawnImpl(
    Util::Invocable<asio::awaitable<void>> auto&& task,
    Util::Invocable<void> auto&& handler,
    asio::thread_pool& pool
) {
    asio::co_spawn(pool, [task]() {
        return task();
    }, [this, handler](std::exception_ptr e) {
        log_exception(e);
        handler();
    });
}

void Scheduler::spawn(
    Util::Invocable<asio::awaitable<void>> auto&& task,
    Util::Invocable<void> auto&& handler
) {
    spawnImpl(std::forward<decltype(task)>(task), std::forward<decltype(handler)>(handler), pool);
}

} // Async

#endif //SCHEDULER_H
