/**
 * \file next_event.h
 * \brief The libinput handler for fetching the next event.
 */

#ifndef EVGETLIBINPUT_NEXT_EVENT_H
#define EVGETLIBINPUT_NEXT_EVENT_H

#include <boost/asio/awaitable.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/scope/unique_fd.hpp>

#include <atomic>
#include <functional>
#include <optional>

#include "evget/error.h"
#include "evget/input_event.h"
#include "evget/next_event.h"
#include "evgetlibinput/libinput.h"

namespace evgetlibinput {

/**
 * \brief A libinput implementation for the `NextEvent` interface.
 */
class NextEvent : public evget::NextEvent<evget::InputEvent<LibInputEvent>> {
public:
    /**
     * \brief Construct a new `NextEvent`.
     * \param libinput_api the libinput API wrapper
     */
    explicit NextEvent(LibInputApi& libinput_api);

    /**
     * \brief Get the next libinput event blocking until arrival.
     * \return the next event, or an error if failed
     */
    [[nodiscard]] boost::asio::awaitable<evget::Result<evget::InputEvent<LibInputEvent>>> Next() const override;

    /**
     * \brief Request a stop.
     */
    void RequestStop();

private:
    [[nodiscard]] boost::asio::awaitable<evget::Result<void>> WaitForInput() const;

    std::reference_wrapper<LibInputApi> libinput_api_;
    std::atomic<bool> stop_requested_{false};
    boost::scope::unique_fd stop_event_;
    mutable std::optional<boost::asio::posix::stream_descriptor> input_;
    mutable std::optional<boost::asio::posix::stream_descriptor> stop_;
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_NEXT_EVENT_H
