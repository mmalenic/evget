/**
 * \file console_stop.h
 * \brief Console control handler that runs a stop on Ctrl+Break and close.
 */

#ifndef EVGETWINDOWS_CONSOLE_STOP_H
#define EVGETWINDOWS_CONSOLE_STOP_H

#include <windows.h>

#include <functional>

namespace evgetwindows {

/**
 * \brief Registers a console control handler that runs a stop callback on Ctrl+Break and console close.
 *
 * Only one instance may exist at a time because the handler routine has no context pointer.
 */
class ConsoleStop {
public:
    /**
     * \brief Store the callback and register the console control handler.
     * \param on_stop the callback which blocks until the stop has finished
     */
    explicit ConsoleStop(std::function<void()> on_stop);

    ConsoleStop(const ConsoleStop&) = delete;
    ConsoleStop(ConsoleStop&&) = delete;
    ConsoleStop& operator=(const ConsoleStop&) = delete;
    ConsoleStop& operator=(ConsoleStop&&) = delete;

    /**
     * \brief Unregister the console control handler and clear the callback.
     */
    ~ConsoleStop();

    /**
     * \brief Run the stop callback for Ctrl+Break and console close.
     * \param ctrl_type the console control event
     * \return `TRUE` if the event was handled, `FALSE` to pass it to the next handler
     */
    static BOOL WINAPI Handle(DWORD ctrl_type);

private:
    bool registered_{false};
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_CONSOLE_STOP_H
