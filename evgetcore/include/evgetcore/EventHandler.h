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

#ifndef EVGET_INCLUDE_EVENTHANDLER_H
#define EVGET_INCLUDE_EVENTHANDLER_H

#include <vector>

#include "EventListener.h"
#include "EventLoop.h"
#include "EventTransformer.h"
#include "Storage.h"
#include "SystemEvent.h"

namespace EvgetCore {

namespace asio = boost::asio;

/**
 * Class represents a listener for events.
 * @tparam T type of data
 */
template <asio::execution::executor E, typename T>
class EventHandler : Task<E>, EventListener<T> {
public:
    /**
     * Create the event listener with storage.
     * @param storage storage
     * @param transformer transformer
     * @param rawEvents systemEvents
     */
    EventHandler(
        E& context,
        Storage<E>& storage,
        // EventTransformer<T>& transformer,
        EventLoop<E, T>& eventLoop
    );

    void notify(T event) override;
    asio::awaitable<void> start() override;

private:
    Storage<E>& storage;
    // EventTransformer<T>& transformer;
    EventLoop<E, T>& eventLoop;
};

template <asio::execution::executor E, typename T>
asio::awaitable<void> EventHandler<E, T>::start() {
    co_await Task<E>::start();
    co_await storage.start();
    co_await eventLoop.eventLoop();
    co_return;
}

template <asio::execution::executor E, typename T>
EventHandler<E, T>::EventHandler(
    E& context,
    Storage<E>& storage,
    // EventTransformer<T>& transformer,
    EventLoop<E, T>& eventLoop
)
    : Task<E>{context}, storage{storage}, eventLoop{eventLoop} {
    eventLoop.registerSystemEventListener(*this);
}

template <asio::execution::executor E, typename T>
void EventHandler<E, T>::notify(T event) {
    // storage.notify(transformer.transformEvent(event));
}
}  // namespace EvgetCore

#endif  // EVGET_INCLUDE_EVENTHANDLER_H
