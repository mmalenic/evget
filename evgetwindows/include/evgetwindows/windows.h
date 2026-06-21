/**
 * \file windows.h
 * \brief The Windows API abstraction for evgetwindows.
 */

#ifndef EVGETWINDOWS_WINDOWS_H
#define EVGETWINDOWS_WINDOWS_H

#include <boost/asio/awaitable.hpp>

#include "evget/error.h"
#include "evgetwindows/raw_event.h"

namespace evgetwindows {

/**
 * \brief API abstraction for the Windows event source.
 */
class WindowsApi {
public:
    WindowsApi() = default;
    virtual ~WindowsApi() = default;

    WindowsApi(WindowsApi&&) noexcept = delete;
    WindowsApi& operator=(WindowsApi&&) noexcept = delete;

    WindowsApi(const WindowsApi&) = delete;
    WindowsApi& operator=(const WindowsApi&) = delete;

    /**
     * \brief Start and register the Raw Input devices loop.
     * \return a void result, or an error if registration fails
     */
    virtual evget::Result<void> Start() = 0;

    /**
     * \brief Stop the message loop and close the channel.
     */
    virtual void Stop() = 0;

    /**
     * \brief Receive the next raw event from the loop.
     * \return the next raw event, or an error if the channel is closed
     */
    virtual boost::asio::awaitable<evget::Result<RawEvent>> ReceiveNext() = 0;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_WINDOWS_H
