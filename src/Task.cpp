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

#include "Task.h"

bool Task::isCancelled() const {
    checkContext();
    return cancelled.load();
}

void Task::cancel() {
    checkContext();
    cancelled.store(true);
}

Task::Task() : context{}, started{false}, cancelled{false}, stopped{false} {
}

bool Task::isStarted() const {
    checkContext();
    return started.load();
}

bool Task::isCreated() const {
    return context.has_value();
}

bool Task::isStopped() const {
    checkContext();
    return started.load();
}

boost::asio::awaitable<void> Task::start() {
    checkContext();
    started.store(true);
}

void Task::stop() {
    checkContext();
    stopped.store(true);
}

void Task::checkContext() const {
    if (!isCreated()) {
        throw UnsupportedOperationException{"Task contains no context."};
    }
}

void Task::create(boost::asio::io_context& context) {
    this->context = context;
}

boost::asio::io_context& Task::getContext() const {
    checkContext();
    return context.get();
}
