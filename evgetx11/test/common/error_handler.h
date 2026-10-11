#ifndef EVGETX11_TEST_COMMON_ERROR_HANDLER_H
#define EVGETX11_TEST_COMMON_ERROR_HANDLER_H

#include <X11/Xlib.h>

#include <array>
#include <cstddef>

#include "evgetx11/error_router.h"

namespace test {

/// \brief The result `HostErrorHandler` returns.
constexpr int kHostHandled = 42;

/**
 * \brief The handler `SetErrorHandler` writes.
 * \return reference to the handler
 */
XErrorHandler& InstalledErrorHandler();

/**
 * \brief Install a handler in `InstalledErrorHandler`.
 * \param handler the handler to install
 * \return the previously installed handler
 */
XErrorHandler SetErrorHandler(XErrorHandler handler);

/**
 * \brief A handler installed before evget.
 * \return `kHostHandled`
 */
int HostErrorHandler(Display* display, XErrorEvent* event);

/**
 * \brief The handler evget installs.
 * \return 0
 */
int RoutedErrorHandler(Display* display, XErrorEvent* event);

/**
 * \brief The handler installed after evget.
 * \return 0
 */
int LaterHostErrorHandler(Display* display, XErrorEvent* event);

/**
 * \brief The router `ChainingErrorHandler` chains to.
 * \return reference to the router slot
 */
evgetx11::ErrorRouter*& ChainedRouter();

/**
 * \brief The number of times `ChainingErrorHandler` ran.
 * \return reference to the count
 */
int& ChainingCalls();

/**
 * \brief A handler an embedding program installed after evget.
 * \return the result of dispatching to `ChainedRouter`
 */
int ChainingErrorHandler(Display* display, XErrorEvent* event);

/**
 * \brief Describe any error as `"fake error"`.
 * \return 0
 */
int FakeErrorText(Display* display, int code, char* buffer, int length);

/**
 * \brief A fake display pointer.
 */
class FakeDisplays {
public:
    /**
     * \brief Get a display pointer.
     * \param index which display
     * \return the display pointer
     */
    [[nodiscard]] Display* At(std::size_t index);

    /// \brief The number of displays.
    static constexpr std::size_t kCount = 3;

private:
    std::array<std::byte, kCount> storage_{};
};

/**
 * \brief Create an error for a request.
 * \param error_code the X error code
 * \param request_code the major request code
 * \return the error
 */
XErrorEvent CreateXErrorEvent(unsigned char error_code, unsigned char request_code);

} // namespace test

#endif
