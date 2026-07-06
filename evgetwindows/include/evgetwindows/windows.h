/**
 * \file windows.h
 * \brief The Windows API abstraction for evgetwindows.
 */

#ifndef EVGETWINDOWS_WINDOWS_H
#define EVGETWINDOWS_WINDOWS_H

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <tuple>

#include "evget/error.h"
#include "evgetwindows/message_window.h"
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
    virtual boost::asio::awaitable<std::tuple<boost::system::error_code, RawEvent>> ReceiveNext() = 0;
};

/**
 * \brief The concrete `WindowsApi` implementation using `MessageWindow`.
 */
class Windows : public WindowsApi {
public:
    /**
     * \brief Create a `MessageWindow` and start the event loop.
     * \param executor the executor the channel completes on
     * \return the API implementation
     */
    static evget::Result<std::unique_ptr<Windows>> New(const boost::asio::any_io_executor& executor);

    Windows(const Windows&) = delete;
    Windows(Windows&&) noexcept = delete;
    Windows& operator=(const Windows&) = delete;
    Windows& operator=(Windows&&) noexcept = delete;
    ~Windows() override = default;

    evget::Result<void> Start() override;
    void Stop() override;
    boost::asio::awaitable<std::tuple<boost::system::error_code, RawEvent>> ReceiveNext() override;

private:
    explicit Windows(const boost::asio::any_io_executor& executor);

    MessageWindow message_window_;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_WINDOWS_H
