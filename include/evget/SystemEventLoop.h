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

#ifndef EVGET_INCLUDE_SYSTEMEVENTLOOP_H
#define EVGET_INCLUDE_SYSTEMEVENTLOOP_H

#include <vector>
#include <chrono>
#include <boost/fiber/buffered_channel.hpp>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include "ShutdownHandler.h"
#include "Task.h"
#include "SystemEvent.h"
#include "EventListener.h"

/**
 * Class represents processing the system events.
 * @tparam T type of events to process
 */
template <boost::asio::execution::executor E, typename T>
class SystemEventLoop : public Task<E>, public EventListener<SystemEvent<T>> {
public:
    /**
     * Create the system events class.
     * @param nDevices number of devices tracked
     */
    SystemEventLoop(E& context, size_t nDevices);

    /**
     * Set up and run the event loop.
     */
    virtual boost::asio::awaitable<void> eventLoop() = 0;

    /**
     * Submit the result of a coroutine.
     * @param result result to submit
     */
    virtual void submitOutcome(bool result);

    /**
     * Register listeners to notify.
     * @param systemEventListener lister
     */
    void registerSystemEventListener(EventListener<SystemEvent<T>>& systemEventListener);

    boost::asio::awaitable<void> start() override;
    void notify(SystemEvent<T> event) override;

    virtual ~SystemEventLoop() = default;
    SystemEventLoop(const SystemEventLoop&) = default;
    SystemEventLoop(SystemEventLoop&&) noexcept = default;
    SystemEventLoop& operator=(const SystemEventLoop&) = default;
    SystemEventLoop& operator=(SystemEventLoop&&) noexcept = default;

private:
    const size_t nDevices;
    std::vector<bool> results;
    std::vector<std::reference_wrapper<EventListener<SystemEvent<T>>>> eventListeners;
};

template <boost::asio::execution::executor E, typename T>
boost::asio::awaitable<void> SystemEventLoop<E, T>::start() {
    co_await Task<E>::start();
    co_await eventLoop();
    this->stop();
    co_return;
}

template <boost::asio::execution::executor E, typename T>
void SystemEventLoop<E, T>::notify(SystemEvent<T> event) {
    for (auto listener : eventListeners) {
        listener.get().notify(event);
    }
}

template <boost::asio::execution::executor E, typename T>
void SystemEventLoop<E, T>::registerSystemEventListener(EventListener<SystemEvent<T>>& systemEventListener) {
    eventListeners.push_back(systemEventListener);
}

template <boost::asio::execution::executor E, typename T>
void SystemEventLoop<E, T>::submitOutcome(bool result) {
    results.push_back(result);
    if (results.size() == nDevices && none_of(results.begin(), results.end(), [](bool v) { return v; })) {
        spdlog::error("No devices were set.");
        throw UnsupportedOperationException();
    }
}

template <boost::asio::execution::executor E, typename T>
SystemEventLoop<E, T>::SystemEventLoop(E& context, size_t nDevices) : Task<E>{context}, nDevices{nDevices}, results{}, eventListeners{} {
}

#endif //EVGET_INCLUDE_SYSTEMEVENTLOOP_H