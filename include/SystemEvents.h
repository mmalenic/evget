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

#ifndef EVGET_INCLUDE_SYSTEMEVENTS_H
#define EVGET_INCLUDE_SYSTEMEVENTS_H

#include <vector>
#include <chrono>
#include <boost/fiber/buffered_channel.hpp>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include "EventHandler.h"
#include "ShutdownHandler.h"
#include "Task.h"

/**
 * Class represents processing the system events.
 * @tparam T type of events to process
 */
template <typename T>
class SystemEvents : public Task {
public:
    /**
     * Create the system events class.
     * @param nDevices number of devices tracked
     */
    explicit SystemEvents(size_t nDevices);

    /**
     * Set up and run the event loop.
     */
    virtual boost::asio::awaitable<void> eventLoop() = 0;

    /**
     * Submit the result of a coroutines.
     * @param result result to submit
     */
    virtual void submitResult(bool result);

    boost::asio::awaitable<void> start() override;

    virtual ~SystemEvents() = default;
    SystemEvents(const SystemEvents&) = default;
    SystemEvents(SystemEvents&&) noexcept = default;
    SystemEvents& operator=(const SystemEvents&) = default;
    SystemEvents& operator=(SystemEvents&&) noexcept = default;

private:
    const size_t nDevices;
    std::vector<bool> results;
};

template<typename T>
boost::asio::awaitable<void> SystemEvents<T>::start() {
    co_await Task::start();
    co_await eventLoop();
    stop();
}

template<typename T>
void SystemEvents<T>::submitResult(bool result) {
    results.push_back(result);
    if (results.size() == nDevices && none_of(results.begin(), results.end(), [](bool v) { return v; })) {
        spdlog::error("No devices were set.");
        throw UnsupportedOperationException();
    }
}

template<typename T>
SystemEvents<T>::SystemEvents(const size_t nDevices) : nDevices{nDevices}, results{} {
}

#endif //EVGET_INCLUDE_SYSTEMEVENTS_H
