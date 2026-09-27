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
 * \brief This creates the X protocol error router.
 */
class ErrorRouter {
public:
    /// \brief Install an error handler and return the previous one.
    using SetHandler = XErrorHandler (*)(XErrorHandler);

    /// \brief Describe an error code in a buffer.
    using ErrorText = int (*)(Display*, int, char*, int);

    /**
     * \brief Create the error router.
     * \param set_handler installs an error handler
     * \param error_text describes an error code
     * \param handler the handler to install
     */
    ErrorRouter(SetHandler set_handler, ErrorText error_text, XErrorHandler handler);

    /**
     * \brief Register the display for the error handler.
     * \param display the display
     */
    void Register(Display* display);

    /**
     * \brief Unregister the display for the error handler.
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
     * \brief Get the router from the Xlib error handler.
     * \return the process router
     */
    static ErrorRouter& Process();

private:
    static int OnProcessError(Display* display, XErrorEvent* event);

    SetHandler set_handler_;
    ErrorText error_text_;
    XErrorHandler handler_;

    std::mutex mutex_;
    std::unordered_map<Display*, std::size_t> displays_;
    XErrorHandler previous_{nullptr};
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
