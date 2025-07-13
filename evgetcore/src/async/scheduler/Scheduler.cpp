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

#include "evgetcore/async/scheduler/Scheduler.h"

#include <boost/asio/awaitable.hpp>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <exception>
#include <utility>

EvgetCore::Scheduler::Scheduler(std::size_t nThreads) : pool{nThreads} {}

void EvgetCore::Scheduler::join() {
    pool.join();
}

void EvgetCore::Scheduler::stop() {
    stopped.store(true);
    pool.stop();
}

boost::asio::awaitable<bool> EvgetCore::Scheduler::isStopped() const {
    co_return stopped.load();
}

boost::asio::thread_pool::executor_type EvgetCore::Scheduler::getExecutor() {
    return pool.get_executor();
}

void EvgetCore::Scheduler::log_exception(const std::exception_ptr& error) {
    try {
        if (error) {
            std::rethrow_exception(error);
        }
    } catch (const std::exception& e) {
        spdlog::error("Exception in coroutine: {}", e.what());
        stop();
    }
}

void EvgetCore::Scheduler::spawn(asio::awaitable<void>&& task) {
    spawnImpl(std::move(task), [this] { this->stop(); }, pool);
}
