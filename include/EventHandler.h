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
#include "Storage.h"
#include "SystemEventListener.h"

/**
 * Class represents a listener for events.
 * @tparam T type of data
 */
template <typename T>
class EventHandler : SystemEventListener {
public:
    /**
     * Create the event listener with storage.
     * @param storage storage
     * @param transformer transformer
     * @param rawEvents rawEvents
     */
    EventHandler(Storage& storage, EventTransformer<T>& transformer, SystemEventLoop<T>& eventLoop);

    void notify(SystemEvent<T> event) override;

    virtual ~EventHandler() = default;
    EventHandler(const EventHandler&) = default;
    EventHandler(EventHandler&&) noexcept = default;
    EventHandler& operator=(const EventHandler&) = default;
    EventHandler& operator=(EventHandler&&) noexcept = default;

private:
    Storage& storage;
    EventTransformer<T>& transformer;
    SystemEventLoop<T>& eventLoop;
};

template<typename T>
EventHandler<T>::EventHandler(Storage& storage, EventTransformer<T>& transformer, SystemEventLoop<T>& eventLoop) :
    storage{storage}, transformer{transformer}, eventLoop{eventLoop} {
    boost::asio::thread_pool context{};

    auto notify = std::bind(&processEvent, &this, std::placeholders::_1);
    eventLoop.context(context);
    eventLoop.eventLoop(notify);
}

template<typename T>
void EventHandler<T>::notify(SystemEvent<T> event) {
    storage.storeEvents(transformer.transformEvent(event));
}

#endif //EVGET_INCLUDE_EVENTHANDLER_H
