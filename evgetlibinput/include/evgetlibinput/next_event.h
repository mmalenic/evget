/**
 * \file next_event.h
 * \brief The libinput handler for fetching the next event.
 */

#ifndef EVGETLIBINPUT_NEXT_EVENT_H
#define EVGETLIBINPUT_NEXT_EVENT_H

#include <functional>

#include "evget/input_event.h"
#include "evget/next_event.h"
#include "evgetlibinput/libinput_api.h"

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

    [[nodiscard]] boost::asio::awaitable<evget::Result<evget::InputEvent<LibInputEvent>>> Next() const override;

private:
    std::reference_wrapper<LibInputApi> libinput_api_;
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_NEXT_EVENT_H
