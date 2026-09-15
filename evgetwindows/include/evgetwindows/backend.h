/**
 * \file backend.h
 * \brief Windows input backend.
 */

#ifndef EVGETWINDOWS_BACKEND_H
#define EVGETWINDOWS_BACKEND_H

#include <boost/asio/any_io_executor.hpp>

#include <memory>

#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/input_event.h"
#include "evget/storage/store.h"
#include "evgetwindows/event_transformer.h"
#include "evgetwindows/hid_query_api.h"
#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/next_event.h"
#include "evgetwindows/windows.h"
#include "evgetwindows/windows_query_api.h"

namespace evgetwindows {

/**
 * \brief Represents the Windows backend.
 */
class Backend {
public:
    /**
     * \brief Create the Windows backend.
     * \param storage the event storage
     * \param executor the executor the channel completes on
     * \return the backend, or an error if the loop fails to start
     */
    static evget::Result<std::unique_ptr<Backend>>
    Create(evget::Store& storage, const boost::asio::any_io_executor& executor);

    /**
     * \brief Get the event handler.
     * \return reference to the event handler
     */
    evget::EventHandler<evget::InputEvent<RawEvent>>& Handler();

    /**
     * \brief Stop the Raw Input source, closing the channel so the event loop exits.
     */
    void Stop();

    Backend(const Backend&) = delete;
    Backend(Backend&&) = delete;
    Backend& operator=(const Backend&) = delete;
    Backend& operator=(Backend&&) = delete;
    ~Backend() = default;

private:
    Backend(std::unique_ptr<WindowsApi> windows, evget::Store& storage);

    std::unique_ptr<WindowsApi> windows_;
    std::unique_ptr<WindowsQueryApi> query_;
    std::unique_ptr<HidQueryApi> hid_query_;
    ModifierTracker tracker_;
    EventTransformer transformer_;
    NextEvent next_event_;
    evget::EventHandler<evget::InputEvent<RawEvent>> handler_;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_BACKEND_H
