// Copyright (C) Marko Malenic 2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef EVGET_INCLUDE_TASK_H
#define EVGET_INCLUDE_TASK_H

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <spdlog/spdlog.h>

#include <atomic>
#include <optional>

#include "UnsupportedOperationException.h"

namespace EvgetCore {

namespace asio = boost::asio;

/**
 * Task class.
 */
template <asio::execution::executor E>
class Task {
public:
    /**
     * Create task object.
     */
    explicit Task(E& context);

    /**
     * Cancel the task.
     */
    void cancel();

    /**
     * Stop the task.
     */
    void stop();

    /**
     * Get context.
     * @return context
     */
    [[nodiscard]] E& getContext() const;

    /**
     * Check if started.
     * @return has started
     */
    [[nodiscard]] bool isStarted() const;

    /**
     * Check if cancelled.
     * @return is cancelled.
     */
    [[nodiscard]] bool isCancelled() const;

    /**
     * Check if stopped normally.
     * @return is stopped.
     */
    [[nodiscard]] bool isStopped() const;

    /**
     * Start the task.
     */
    virtual asio::awaitable<void> start();

private:
    std::reference_wrapper<E> executionContext;

    std::atomic<bool> started;
    std::atomic<bool> cancelled;
    std::atomic<bool> stopped;
};

template <asio::execution::executor E>
bool Task<E>::isCancelled() const {
    return cancelled.load();
}

template <asio::execution::executor E>
void Task<E>::cancel() {
    cancelled.store(true);
}

template <asio::execution::executor E>
Task<E>::Task(E& context) : executionContext{context}, started{false}, cancelled{false}, stopped{false} {}

template <asio::execution::executor E>
bool Task<E>::isStarted() const {
    return started.load();
}

template <asio::execution::executor E>
bool Task<E>::isStopped() const {
    return started.load();
}

template <asio::execution::executor E>
asio::awaitable<void> Task<E>::start() {
    started.store(true);
    co_return;
}

template <asio::execution::executor E>
void Task<E>::stop() {
    stopped.store(true);
}

template <asio::execution::executor E>
E& Task<E>::getContext() const {
    return executionContext;
}

}  // namespace EvgetCore

#endif  // EVGET_INCLUDE_TASK_H
