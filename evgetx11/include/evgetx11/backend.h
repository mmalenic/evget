/**
 * \file backend.h
 * \brief Owns all X11 backend objects.
 */

#ifndef EVGETX11_BACKEND_H
#define EVGETX11_BACKEND_H

#include <X11/Xlib.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/event_transformer.h"
#include "evget/storage/store.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"
#include "evgetx11/x11.h"

namespace evgetx11 {

/**
 * \brief Create an X11 backend, used for convenience to store all objects in the same place.
 */
class Backend {
public:
    /**
     * \brief Create the X11 backend.
     * \param storage the event storage
     * \param display optional X11 display name to connect to. If unset, the default display
     *        is used.
     * \return the backend
     */
    static evget::Result<std::unique_ptr<Backend>>
    Create(evget::Store& storage, const std::optional<std::string>& display);

    /**
     * \brief Create the X11 backend that reports when the connection to the X server is lost.
     * \param storage the event storage
     * \param display X11 display name to connect to, default display is usedif unset
     * \param on_connection_lost runs on connection lost
     * \return the backend
     */
    static evget::Result<std::unique_ptr<Backend>>
    Create(evget::Store& storage, const std::optional<std::string>& display, std::function<void()> on_connection_lost);

    /**
     * \brief Get the event handler.
     * \return reference to the event handler, or an error if the backend was not created
     *         through \ref Create.
     */
    evget::Result<std::reference_wrapper<evget::EventHandler<InputEvent>>> Handler();

    /**
     * \brief Request a stop for the X11 input handler.
     */
    void RequestStop();

    Backend(const Backend&) = delete;
    Backend(Backend&&) = delete;
    Backend& operator=(const Backend&) = delete;
    Backend& operator=(Backend&&) = delete;
    ~Backend() = default;

private:
    Backend(std::unique_ptr<Display, decltype(&XCloseDisplay)> display, std::function<void()> on_connection_lost);

    std::unique_ptr<Display, decltype(&XCloseDisplay)> display_;
    X11 api_;
    std::unique_ptr<evget::EventTransformer<InputEvent>> transformer_;
    std::unique_ptr<InputHandler> next_event_;
    std::optional<evget::EventHandler<InputEvent>> handler_;
};

} // namespace evgetx11

#endif // EVGETX11_BACKEND_H
