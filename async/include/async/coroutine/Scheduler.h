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
#include <async/Util.h>
#include <spdlog/spdlog.h>

namespace Async {

namespace asio = boost::asio;

class Scheduler {
public:
    Scheduler() = default;
    explicit Scheduler(std::size_t nThreads);

    void spawn(Invocable<asio::awaitable<void>> auto&& task, Invocable<void, std::exception_ptr, const Scheduler&> auto&& handler);

    template<typename T>
    void spawn(Invocable<asio::awaitable<T>> auto&& task, Invocable<void, std::exception_ptr, T, const Scheduler&> auto&& handler);

    void join();
    void stop();

private:
    asio::thread_pool pool;

    void log_exception(std::exception_ptr e);
};

void Scheduler::spawn(
    Invocable<asio::awaitable<void>> auto&& task,
    Invocable<void, std::exception_ptr, const Scheduler&> auto&& handler
) {
    asio::co_spawn(pool, task, [this, handler](std::exception_ptr e) {
        log_exception(e);
        handler(e, *this);
    });
}

template <typename T>
void Scheduler::spawn(
    Invocable<asio::awaitable<T>> auto&& task,
    Invocable<void, std::exception_ptr, T, const Scheduler&> auto&& handler
) {
    asio::co_spawn(pool, task, [this, handler](std::exception_ptr e, T value) {
        log_exception(e);
        handler(e, value, *this);
    });
}

} // Async

#endif //SCHEDULER_H
