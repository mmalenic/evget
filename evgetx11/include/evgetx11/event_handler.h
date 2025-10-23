/**
 * \file event_handler.h
 * \brief X11-specific event handler implementation for processing input events.
 */

#ifndef EVGETX11_EVENT_HANDLER_H
#define EVGETX11_EVENT_HANDLER_H

#include "event_transformer.h"
#include "evget/event_handler.h"
#include "evget/event_listener.h"

namespace evgetx11 {
/**
 * \brief X11-specific event handler that processes `InputEvent` objects.
 */
class EventHandler : public evget::EventListener<InputEvent> {
public:
    /**
     * \brief Construct an EventHandler with storage, event loop, and transformer.
     * \param storage reference to the storage system
     * \param event_loop unique pointer to the event loop
     * \param transformer unique pointer to the event transformer
     */
    EventHandler(
        evget::Store& storage,
        std::unique_ptr<evget::EventLoop<InputEvent>> event_loop,
        std::unique_ptr<evget::EventTransformer<InputEvent>> transformer
    );

    /**
     * \brief Handle notification of a new input event.
     * \param event the input event to process
     * \return awaitable result indicating success or failure
     */
    boost::asio::awaitable<evget::Result<void>> Notify(InputEvent event) override;

    /**
     * \brief Start the event handler processing.
     * \return awaitable result indicating success or failure
     */
    boost::asio::awaitable<evget::Result<void>> Start() override;

    /**
     * \brief Stop the event handler processing.
     */
    void Stop();

private:
    std::unique_ptr<evget::EventTransformer<InputEvent>> transformer_;
    std::unique_ptr<evget::EventLoop<InputEvent>> event_loop_;
    evget::EventHandler<InputEvent> handler_;
};

/**
 * \brief Builder class for constructing EventHandler instances with specific configurations.
 */
class EventHandlerBuilder {
public:
    /**
     * \brief Configure the builder to handle pointer and key events.
     * \param x_wrapper reference to the X11 API wrapper
     * \return reference to this builder
     */
    EventHandlerBuilder& PointerKey(X11Api& x_wrapper);

    /**
     * \brief Configure the builder to handle touch events.
     * \return reference to this builder
     */
    EventHandlerBuilder& Touch();

    /**
     * \brief Build the `EventHandler` with the configured settings.
     * \param storage reference to the storage system
     * \param x11_api reference to the X11 API
     * \return result containing the constructed `EventHandler` or an error
     */
    evget::Result<EventHandler> Build(evget::Store& storage, X11Api& x11_api) &&;

private:
    EventTransformerBuilder builder_;
};
} // namespace evgetx11

#endif
