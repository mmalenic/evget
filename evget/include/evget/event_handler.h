/**
 * \file event_handler.h
 * \brief Event handler for processing and storing input events.
 */

#ifndef EVGET_EVENT_HANDLER_H
#define EVGET_EVENT_HANDLER_H

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <utility>

#include "evget/error.h"
#include "evget/event_listener.h"
#include "evget/event_loop.h"
#include "evget/event_transformer.h"
#include "evget/storage/store.h"

namespace evget {

/**
 * \brief Class represents a listener for events.
 * \tparam T type of data
 */
template <typename T>
class EventHandler : public EventListener<T> {
public:
    /**
     * \brief Create the event listener with storage.
     * \param storage storage
     * \param transformer transformer
     * \param next_event the next event interface
     */
    EventHandler(Store& storage, EventTransformer<T>& transformer, NextEvent<T>& next_event);

    boost::asio::awaitable<Result<void>> Notify(T event) override;
    boost::asio::awaitable<Result<void>> Start() override;

    /**
     * \brief Stop the event handler processing.
     */
    void Stop();

private:
    std::reference_wrapper<Store> storage_;
    std::reference_wrapper<EventTransformer<T>> transformer_;
    std::reference_wrapper<NextEvent<T>> next_event_;
    EventLoop<T> event_loop_;
};

template <typename T>
boost::asio::awaitable<Result<void>> EventHandler<T>::Start() {
    co_return co_await event_loop_.Start();
}

template <typename T>
void EventHandler<T>::Stop() {
    event_loop_.Stop();
}

template <typename T>
EventHandler<T>::EventHandler(Store& storage, EventTransformer<T>& transformer, NextEvent<T>& next_event)
    : storage_{storage}, transformer_{transformer}, next_event_{next_event}, event_loop_{next_event, {{*this}}} {}

template <typename T>
boost::asio::awaitable<Result<void>> EventHandler<T>::Notify(T event) {
    co_return storage_.get().StoreEvent(transformer_.get().TransformEvent(std::move(event)));
}
} // namespace evget

#endif
