#ifndef EVGET_EVENT_LOOP_H
#define EVGET_EVENT_LOOP_H

#include "evget/event_listener.h"

namespace evget {

/**
 * Class represents processing events.
 * \tparam T type of events to process
 */
template <typename T>
class EventLoop : public EventListener<T> {
public:
    /**
     * Register listeners to notify.
     * \param event_listener listener
     */
    virtual void RegisterEventListener(EventListener<T>& event_listener) = 0;

    /**
     * Stop the event loop.
     */
    virtual void Stop() = 0;

    EventLoop() = default;

    ~EventLoop() override = default;

    EventLoop(EventLoop&&) noexcept = delete;
    EventLoop& operator=(EventLoop&&) noexcept = delete;

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
};

} // namespace evget

#endif
