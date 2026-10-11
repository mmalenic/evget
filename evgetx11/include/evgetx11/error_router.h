/**
 * \file error_router.h
 * \brief X protocol error handling for evget.
 */

#ifndef EVGETX11_ERROR_ROUTER_H
#define EVGETX11_ERROR_ROUTER_H

#include <X11/Xlib.h>

#include <cstddef>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "evgetx11/log_once.h"

namespace evgetx11 {

/**
 * \brief Installs a process based Xlib handler.
 *
 * Not thread-safe as `ErrorRouter` serialises every call.
 *
 * \tparam Handler the handler type, `i.e. `XErrorHandler` or `XIOErrorHandler`
 */
template <typename Handler>
class HandlerChain {
public:
    /// \brief Installs the process based handler.
    using SetHandler = Handler (*)(Handler);

    /**
     * \brief Create a chain.
     * \param set_handler installs a process based handler
     * \param handler the handler
     */
    HandlerChain(SetHandler set_handler, Handler handler);

    /**
     * \brief Install the handler.
     */
    void Install();

    /**
     * \brief Restore the replaced handler.
     */
    void Restore();

    /**
     * \brief Get the previous handler to forward to.
     * \return the replaced handler
     */
    [[nodiscard]] Handler Previous() const;

private:
    SetHandler set_handler_;
    Handler handler_;
    Handler previous_{nullptr};
    Handler left_in_place_{nullptr};
};

/**
 * \brief This creates the X protocol error router.
 */
class ErrorRouter {
public:
    /// \brief Install an error handler and return the previous one.
    using SetHandler = XErrorHandler (*)(XErrorHandler);
    /// \brief Installs an error handler and return the previous one.
    using SetHandler = HandlerChain<XErrorHandler>::SetHandler;

    /// \brief Installs a process based IO error handler.
    using SetIoHandler = HandlerChain<XIOErrorHandler>::SetHandler;

    /// \brief Describe an error code in a buffer.
    using ErrorText = int (*)(Display*, int, char*, int);

    /**
     * \brief Create the error router.
     * \param set_handler installs an error handler
     * \param error_text describes an error code
     * \param handler the handler to install
     */
    ErrorRouter(
        SetHandler set_handler,
        ErrorText error_text,
        XErrorHandler handler,
        SetIoHandler set_io_handler,
        XIOErrorHandler io_handler
    );

    /**
     * \brief Register the display for the error handler.
     * \param display the display
     */
    void Register(Display* display);

    /**
     * \brief Intercept errors on a display, installing the handlers if no display was registered.
     * \param display the display, used only as an id
     * \param on_connection_lost runs when the connection to the display is lost
     */
    void Register(Display* display, std::function<void()> on_connection_lost);

    /**
     * \brief Unregister the display for the error handlers.
     * \param display a display
     */
    void Unregister(Display* display);

    /**
     * \brief Handle an error.
     * \param display the display the error occurred on
     * \param event the error
     * \return zero for a registered display or when there is no handler, otherwise the previous handler result
     */
    int Dispatch(Display* display, XErrorEvent* event);

    /**
     * \brief Handle a lost connection by running the callback of a registered display.
     * \param display the display
     * \return zero when there is no previous handler, otherwise the previous handler result
     */
    int DispatchIo(Display* display);

    /**
     * \brief Get the router from the Xlib error handlers.
     * \return the process router
     */
    static ErrorRouter& Process();

private:
    static int OnProcessError(Display* display, XErrorEvent* event);
    static int OnProcessIoError(Display* display);

    ErrorText error_text_;

    std::mutex mutex_;
    HandlerChain<XErrorHandler> errors_;
    HandlerChain<XIOErrorHandler> io_errors_;
    std::unordered_map<Display*, std::size_t> displays_;
    std::unordered_map<Display*, std::function<void()>> on_connection_lost_;
    LogOnce log_;
};

/**
 * \brief Register a display with an `ErrorRouter`.
 */
class ErrorRegistration {
public:
    /**
     * \brief Register a display.
     * \param router the router to register with
     * \param display the display
     */
    ErrorRegistration(ErrorRouter& router, Display* display);

    /**
     * \brief Register a display.
     * \param router the router to register with
     * \param display the display
     * \param on_connection_lost runs when the connection to the display is lost
     */
    ErrorRegistration(ErrorRouter& router, Display* display, std::function<void()> on_connection_lost);

    /**
     * \brief Unregister the display.
     */
    ~ErrorRegistration();

    ErrorRegistration(const ErrorRegistration&) = delete;
    ErrorRegistration(ErrorRegistration&&) = delete;
    ErrorRegistration& operator=(const ErrorRegistration&) = delete;
    ErrorRegistration& operator=(ErrorRegistration&&) = delete;

private:
    std::reference_wrapper<ErrorRouter> router_;
    Display* display_;
};

} // namespace evgetx11

#endif
