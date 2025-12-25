/**
 * \file next_event.h
 * \brief An interface that represents fetching the next event in the queue.
 */

#ifndef EVGET_NEXT_EVENT_H
#define EVGET_NEXT_EVENT_H

#include <boost/asio/awaitable.hpp>

#include "evget/error.h"

namespace evget {

/**
 * \brief An interface that fetches the next event to process.
 * \tparam T event type
 */
template <typename T>
class NextEvent {
public:
    /**
     * \brief Get the next event.
     * \return the next event
     */
    [[nodiscard]] virtual boost::asio::awaitable<Result<T>> Next() const = 0;

    NextEvent() = default;

    virtual ~NextEvent() = default;

    NextEvent(const NextEvent&) = delete;
    NextEvent(NextEvent&&) noexcept = delete;
    NextEvent& operator=(const NextEvent&) = delete;
    NextEvent& operator=(NextEvent&&) noexcept = delete;
};

} // namespace evget

#endif // EVGET_NEXT_EVENT_H
