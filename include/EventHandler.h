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
#include "EventTransformer.h"
#include "SystemEvent.h"
#include "SystemEventLoop.h"
#include "Storage.h"
#include "EventListener.h"

/**
 * Class represents a listener for events.
 * @tparam T type of data
 */
template <typename T, boost::asio::execution::executor E>
class EventHandler : Task<E>, EventListener<SystemEvent<T>> {
public:
    /**
     * Create the event listener with storage.
     * @param storage storage
     * @param transformer transformer
     * @param rawEvents rawEvents
     */
    EventHandler(E& context, Storage<E>& storage, EventTransformer<T>& transformer, SystemEventLoop<T, E>& eventLoop);

    void notify(SystemEvent<T> event) override;
    boost::asio::awaitable<void> start() override;

    virtual ~EventHandler() = default;
    EventHandler(const EventHandler&) = default;
    EventHandler(EventHandler&&) noexcept = default;
    EventHandler& operator=(const EventHandler&) = default;
    EventHandler& operator=(EventHandler&&) noexcept = default;

private:
    Storage<E>& storage;
    EventTransformer<T>& transformer;
    SystemEventLoop<T, E>& eventLoop;
};

template<typename T, boost::asio::execution::executor E>
boost::asio::awaitable<void> EventHandler<T, E>::start() {
    Task<E>::start();
    storage.start();
    eventLoop.start();
}

template<typename T, boost::asio::execution::executor E>
EventHandler<T, E>::EventHandler(E& context, Storage<E>& storage, EventTransformer<T>& transformer, SystemEventLoop<T, E>& eventLoop) : Task<E>{context},
    storage{storage}, transformer{transformer}, eventLoop{eventLoop} {
    eventLoop.registerSystemEventListener(&this);
}

template<typename T, boost::asio::execution::executor E>
void EventHandler<T, E>::notify(SystemEvent<T> event) {
    storage.notify(transformer.transformEvent(event));
}

#endif //EVGET_INCLUDE_EVENTHANDLER_H
