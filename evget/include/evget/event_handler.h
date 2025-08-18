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

class Scheduler;

namespace asio = boost::asio;

/**
 * Class represents a listener for events.
 * @tparam T type of data
 */
template <typename T>
class EventHandler : EventListener<T> {
public:
    /**
     * Create the event listener with storage.
     * @param storage storage
     * @param transformer transformer
     * @param eventLoop event loop
     */
    EventHandler(Store& storage, EventTransformer<T>& transformer, EventLoop<T>& event_loop);

    Result<void> Notify(T event) override;
    asio::awaitable<Result<void>> Start() override;

private:
    std::reference_wrapper<Store> storage_;
    std::reference_wrapper<EventTransformer<T>> transformer_;
    std::reference_wrapper<EventLoop<T>> event_loop_;
};

template <typename T>
asio::awaitable<Result<void>> EventHandler<T>::Start() {
    co_return co_await event_loop_.get().start();
}

template <typename T>
EventHandler<T>::EventHandler(Store& storage, EventTransformer<T>& transformer, EventLoop<T>& event_loop)
    : storage_{storage}, transformer_{transformer}, event_loop_{event_loop} {
    event_loop.registerEventListener(*this);
}

template <typename T>
Result<void> EventHandler<T>::Notify(T event) {
    return storage_.get().StoreEvent(transformer_.get().transformEvent(std::move(event)));
}
}  // namespace evget

#endif
