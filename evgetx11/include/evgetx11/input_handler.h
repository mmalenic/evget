/**
 * \file input_handler.h
 * \brief X11 input handler for managing event masks and retrieving input events.
 */

#ifndef EVGETX11_INPUT_HANDLER_H
#define EVGETX11_INPUT_HANDLER_H

#include <boost/asio/awaitable.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/scope/unique_fd.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <optional>

#include "evget/error.h"
#include "evget/next_event.h"
#include "evgetx11/input_event.h"
#include "evgetx11/x11.h"

namespace evgetx11 {

/**
 * \brief Handles X11 input events by wrapping the X11 API. This class sets
 * event masks and checks that the X11 API is at least version 2.2.
 */
class InputHandler : public evget::NextEvent<InputEvent> {
public:
    /**
     * \brief Construct an `InputHandler` with an X11 API wrapper.
     * \param x_wrapper reference to the X11 API wrapper
     */
    explicit InputHandler(X11Api& x_wrapper);

    /**
     * \brief Get the next input event from the X11 system waiting until it arrives.
     * \return the next input event
     */
    [[nodiscard]] boost::asio::awaitable<evget::Result<InputEvent>> Next() const override;

    /**
     * \brief Request a stop for the input handler.
     */
    void RequestStop();

private:
    [[nodiscard]] boost::asio::awaitable<evget::Result<void>> WaitForInput() const;

    std::reference_wrapper<X11Api> x_wrapper_;
    std::atomic<bool> stop_requested_{false};
    boost::scope::unique_fd stop_event_;
    mutable std::optional<boost::asio::posix::stream_descriptor> connection_;
    mutable std::optional<boost::asio::posix::stream_descriptor> stop_;
    mutable std::optional<int> remaining_after_stop_;
};

/**
 * \brief Builder class for constructing `InputHandler` instances.
 */
class InputHandlerBuilder {
public:
    /**
     * \brief Build an `InputHandler` with the specified X11 API wrapper.
     * \param x_wrapper reference to the X11 API wrapper
     * \return result containing the constructed InputHandler or an error
     */
    static evget::Result<std::unique_ptr<InputHandler>> Build(X11Api& x_wrapper);
};
} // namespace evgetx11

#endif
