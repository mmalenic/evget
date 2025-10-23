/**
 * \file event_listener.h
 * \brief Abstract interface for event notification and processing.
 */

#ifndef EVGET_EVENT_LISTENER_H
#define EVGET_EVENT_LISTENER_H

#include <boost/asio/awaitable.hpp>

#include "evget/error.h"

namespace evget {

/**
 * \brief Represents event listeners.
 * \tparam T event type
 */
template <typename T>
class EventListener {
public:
    /**
     * \brief Notify if an event.
     * \param event event
     */
    virtual boost::asio::awaitable<Result<void>> Notify(T event) = 0;

    /**
     * \brief Start the listener processing.
     */
    virtual boost::asio::awaitable<Result<void>> Start() = 0;

    EventListener() = default;

    virtual ~EventListener() = default;

    EventListener(const EventListener&) = delete;
    EventListener(EventListener&&) noexcept = delete;
    EventListener& operator=(const EventListener&) = delete;
    EventListener& operator=(EventListener&&) noexcept = delete;
};
} // namespace evget

#endif
