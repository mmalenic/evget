/**
 * \file event_loop.h
 * \brief The event loop for processing input events.
 */

#ifndef EVGET_EVENT_LOOP_H
#define EVGET_EVENT_LOOP_H

#include <memory>

#include "evget/event_listener.h"
#include "evget/next_event.h"

namespace evget {
/**
 * \brief Class represents processing events.
 * \tparam T type of events to process
 */
template <typename T>
class EventLoop {
public:
    /**
     * \brief Create a new event loop.
     * \param next_event next event interface
     * \param listeners the listeners to notify
     */
    EventLoop(NextEvent<T>& next_event, std::vector<std::reference_wrapper<evget::EventListener<T>>> listeners);

    /**
     * \brief Start processing events and notify listeners.
     */
    boost::asio::awaitable<evget::Result<void>> Start();

    /**
     * \brief Stop the event loop.
     */
    void Stop();

    /**
     * \brief Check if the event loop has been stopped.
     * \return awaitable boolean indicating if the loop is stopped
     */
    boost::asio::awaitable<bool> IsStopped();

private:
    std::reference_wrapper<NextEvent<T>> next_event_;
    std::vector<std::reference_wrapper<evget::EventListener<T>>> listeners_;

    bool stopped_{false};
};
} // namespace evget

template <typename T>
evget::EventLoop<T>::EventLoop(
    NextEvent<T>& next_event,
    std::vector<std::reference_wrapper<EventListener<T>>> listeners
)
    : next_event_{next_event}, listeners_{std::move(listeners)} {}

template <typename T>
boost::asio::awaitable<evget::Result<void>> evget::EventLoop<T>::Start() {
    while (!co_await IsStopped()) {
        for (auto& listener : listeners_) {
            auto event = co_await next_event_.get().Next();
            if (!event.has_value()) {
                co_return Err{event.error()};
            }

            auto result = co_await listener.get().Notify(std::move(*event));
            if (!result.has_value()) {
                co_return Err{result.error()};
            }
        }
    }

    co_return evget::Result<void>{};
}

template <typename T>
void evget::EventLoop<T>::Stop() {
    stopped_ = true;
}

template <typename T>
boost::asio::awaitable<bool> evget::EventLoop<T>::IsStopped() {
    co_return stopped_;
}

#endif
