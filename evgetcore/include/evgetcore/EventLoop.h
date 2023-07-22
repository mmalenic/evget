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

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <vector>

#include "EventListener.h"

namespace EvgetCore {

namespace asio = boost::asio;

/**
 * Class represents processing events.
 * @tparam T type of events to process
 */
template <typename T>
class EventLoop : public EventListener<T> {
public:
    /**
     * Set up and run the event loop.
     */
    virtual asio::awaitable<void> eventLoop() = 0;

    /**
     * Register listeners to notify.
     * @param eventListener lister
     */
    virtual void registerEventListener(EventListener<T>& eventListener) = 0;

    /**
     * Stop the event loop.
     */
    virtual void stop() = 0;

    EventLoop() = default;

    virtual ~EventLoop() = default;

    EventLoop(EventLoop&&) noexcept = delete;
    EventLoop& operator=(EventLoop&&) noexcept = delete;

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
};

}  // namespace EvgetCore

#endif  // EVGET_INCLUDE_SYSTEMEVENTLOOP_H
