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

template<boost::asio::execution::executor E>
bool Task<E>::isCancelled() const {
    return cancelled.load();
}

template<boost::asio::execution::executor E>
void Task<E>::cancel() {
    cancelled.store(true);
}

template<boost::asio::execution::executor E>
Task<E>::Task(E& context) : executionContext{context}, started{false}, cancelled{false}, stopped{false} {
}

template<boost::asio::execution::executor E>
bool Task<E>::isStarted() const {
    return started.load();
}

template<boost::asio::execution::executor E>
bool Task<E>::isStopped() const {
    return started.load();
}

template<boost::asio::execution::executor E>
boost::asio::awaitable<void> Task<E>::start() {
    started.store(true);
}

template<boost::asio::execution::executor E>
void Task<E>::stop() {
    stopped.store(true);
}

template<boost::asio::execution::executor E>
E& Task<E>::getContext() const {
    return executionContext;
}