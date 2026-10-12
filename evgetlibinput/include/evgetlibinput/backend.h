/**
 * \file backend.h
 * \brief Owns all libinput backend objects.
 */

#ifndef EVGETLIBINPUT_BACKEND_H
#define EVGETLIBINPUT_BACKEND_H

#include <boost/scope/unique_fd.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

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
     * \brief Create the libinput backend, with tap-to-click turned on.
     * \param dimensions screen dimensions (width, height)
     * \param storage the event storage
     * \param seat optional udev seat name to assign. If unset, `seat0` is used.
     * \return the backend
     */
    static evget::Result<std::unique_ptr<Backend>> Create(
        std::optional<std::pair<std::uint32_t, std::uint32_t>> dimensions,
        evget::Store& storage,
        const std::optional<std::string>& seat
    );

    /**
     * \brief Create the libinput backend.
     * \param dimensions screen dimensions (width, height)
     * \param storage the event storage
     * \param seat optional udev seat name to assign. If unset, `seat0` is used.
     * \param tap_to_click whether to turn tap-to-click on for every touchpad, or keep libinput's default
     * \return the backend
     */
    static evget::Result<std::unique_ptr<Backend>> Create(
        std::optional<std::pair<std::uint32_t, std::uint32_t>> dimensions,
        evget::Store& storage,
        const std::optional<std::string>& seat,
        bool tap_to_click
    );

    /**
     * \brief Get the event handler.
     * \return reference to the event handler
     */
    evget::EventHandler<evget::InputEvent<LibInputEvent>>& Handler();

    /**
     * \brief Request a stop for the libinput handler.
     */
    void RequestStop();

    Backend(const Backend&) = delete;
    Backend(Backend&&) = delete;
    Backend& operator=(const Backend&) = delete;
    Backend& operator=(Backend&&) = delete;
    ~Backend() = default;

private:
    Backend(
        std::unique_ptr<LibInputApi> libinput,
        XkbCommon xkb,
        ScreenDimensions dimensions,
        boost::scope::unique_fd stop_event,
        evget::Store& storage
    );

    std::unique_ptr<LibInputApi> libinput_;
    XkbCommon xkb_;
    EventTransformer transformer_;
    NextEvent next_event_;
    evget::EventHandler<evget::InputEvent<LibInputEvent>> handler_;
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_BACKEND_H
