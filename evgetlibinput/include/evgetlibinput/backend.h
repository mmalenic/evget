/**
 * \file backend.h
 * \brief Owns all libinput backend objects.
 */

#ifndef EVGETLIBINPUT_BACKEND_H
#define EVGETLIBINPUT_BACKEND_H

#include <memory>

#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/input_event.h"
#include "evget/storage/store.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/event_transformer.h"
#include "evgetlibinput/libinput.h"
#include "evgetlibinput/next_event.h"
#include "evgetlibinput/xkbcommon.h"

namespace evgetlibinput {

/**
 * \brief Create a LibInput backend. Used for convenience to store all required objects in a single place.
 */
class Backend {
public:
    /**
     * \brief Create the libinput backend.
     * \param dimensions the screen dimensions
     * \param storage the event storage
     * \return the backend
     */
    static evget::Result<std::unique_ptr<Backend>> Create(ScreenDimensions dimensions, evget::Store& storage);

    /**
     * \brief Get the event handler.
     * \return reference to the event handler
     */
    evget::EventHandler<evget::InputEvent<LibInputEvent>>& Handler();

    Backend(const Backend&) = delete;
    Backend(Backend&&) = delete;
    Backend& operator=(const Backend&) = delete;
    Backend& operator=(Backend&&) = delete;
    ~Backend() = default;

private:
    Backend(std::unique_ptr<LibInputApi> libinput, XkbCommon xkb, ScreenDimensions dimensions, evget::Store& storage);

    std::unique_ptr<LibInputApi> libinput_;
    XkbCommon xkb_;
    EventTransformer transformer_;
    NextEvent next_event_;
    evget::EventHandler<evget::InputEvent<LibInputEvent>> handler_;
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_BACKEND_H
