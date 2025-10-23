/**
 * \file event_loop.h
 * \brief X11-specific event loop implementation for processing system events.
 */

#ifndef EVGETX11_EVENT_LOOP_H
#define EVGETX11_EVENT_LOOP_H

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <optional>

#include "evget/error.h"
#include "evget/event_listener.h"
#include "evget/event_loop.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"

namespace evgetx11 {

/**
 * \brief Class represents processing X11 events.
 */
class EventLoop : public evget::EventLoop<InputEvent> {
public:
    /**
     * \brief Create the system events loop with an input handler.
     * \param x_input_handler the input handler for processing X11 input events
     */
    explicit EventLoop(InputHandler x_input_handler);

    /**
     * \brief Handle notification of a new input event.
     * \param event the input event to process
     * \return awaitable result indicating success or failure
     */
    boost::asio::awaitable<evget::Result<void>> Notify(InputEvent event) override;

    /**
     * \brief Register an event listener to receive notifications.
     * \param event_listener reference to the event listener to register
     */
    void RegisterEventListener(EventListener& event_listener) override;

    /**
     * \brief Stop the event loop processing.
     */
    void Stop() override;

    /**
     * \brief Start the event loop processing.
     * \return awaitable result indicating success or failure
     */
    boost::asio::awaitable<evget::Result<void>> Start() override;

    /**
     * \brief Check if the event loop has been stopped.
     * \return awaitable boolean indicating if the loop is stopped
     */
    boost::asio::awaitable<bool> IsStopped();

private:
    std::optional<std::reference_wrapper<EventListener<InputEvent>>> event_listener_;
    InputHandler handler_;

    bool stopped_{false};
};

/**
 * \brief Builder class for constructing EventLoop instances.
 */
class EventLoopBuilder {
public:
    /**
     * \brief Build an `EventLoop` with the specified input handler.
     * \param input_handler the input handler to use for processing events
     * \return unique pointer to the constructed EventLoop
     */
    static std::unique_ptr<EventLoop> Build(InputHandler input_handler);
};
} // namespace evgetx11

#endif
