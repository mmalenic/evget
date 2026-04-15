/**
 * \file backend.h
 * \brief Owns all X11 backend objects.
 */

#ifndef EVGETX11_BACKEND_H
#define EVGETX11_BACKEND_H

#include <X11/Xlib.h>

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
     * \brief Get the event handler.
     * \return reference to the event handler
     */
    evget::EventHandler<InputEvent>& Handler();

    Backend(const Backend&) = delete;
    Backend(Backend&&) = delete;
    Backend& operator=(const Backend&) = delete;
    Backend& operator=(Backend&&) = delete;
    ~Backend() = default;

private:
    explicit Backend(std::unique_ptr<Display, decltype(&XCloseDisplay)> display);

    std::unique_ptr<Display, decltype(&XCloseDisplay)> display_;
    X11 api_;
    std::unique_ptr<evget::EventTransformer<InputEvent>> transformer_;
    std::unique_ptr<InputHandler> next_event_;
    std::optional<evget::EventHandler<InputEvent>> handler_;
};

} // namespace evgetx11

#endif // EVGETX11_BACKEND_H
