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
 * Class represents a listener for events.
 * @tparam T type of data
 */
template <typename T>
class EventHandler : public EventListener<T> {
public:
    /**
     * Create the event listener with storage.
     * @param storage storage
     * @param transformer transformer
     * @param eventLoop event loop
     */
    EventHandler(Store& storage, EventTransformer<T>& transformer, EventLoop<T>& event_loop);

    boost::asio::awaitable<Result<void>> Notify(T event) override;
    boost::asio::awaitable<Result<void>> Start() override;
    void Stop();

private:
    std::reference_wrapper<Store> storage_;
    std::reference_wrapper<EventTransformer<T>> transformer_;
    std::reference_wrapper<EventLoop<T>> event_loop_;
};

template <typename T>
boost::asio::awaitable<Result<void>> EventHandler<T>::Start() {
    // NOLINTBEGIN(clang-analyzer-core.CallAndMessage)
    co_return co_await event_loop_.get().Start();
    // NOLINTEND(clang-analyzer-core.CallAndMessage)
}

template <typename T>
void EventHandler<T>::Stop() {
    event_loop_.get().Stop();
}

template <typename T>
EventHandler<T>::EventHandler(Store& storage, EventTransformer<T>& transformer, EventLoop<T>& event_loop)
    : storage_{storage}, transformer_{transformer}, event_loop_{event_loop} {
    event_loop.RegisterEventListener(*this);
}

template <typename T>
boost::asio::awaitable<Result<void>> EventHandler<T>::Notify(T event) {
    co_return storage_.get().StoreEvent(transformer_.get().TransformEvent(std::move(event)));
}
} // namespace evget

#endif
