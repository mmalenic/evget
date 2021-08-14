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

#include <atomic>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <UnsupportedOperationException.h>
#include <optional>

/**
 * Task class.
 */
template<boost::asio::execution::executor E>
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
    virtual boost::asio::awaitable<void> start();

private:
    E& executionContext;

    std::atomic<bool> started;
    std::atomic<bool> cancelled;
    std::atomic<bool> stopped;
};

#endif //EVGET_INCLUDE_TASK_H
