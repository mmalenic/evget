/**
 * \file next_event.h
 * \brief The Windows handler for fetching the next event.
 */

#ifndef EVGETWINDOWS_NEXT_EVENT_H
#define EVGETWINDOWS_NEXT_EVENT_H

#include <functional>

#include "evget/input_event.h"
#include "evget/next_event.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"

namespace evgetwindows {

/**
 * \brief The wWindows implementation for `NextEvent.
 */
class NextEvent : public evget::NextEvent<evget::InputEvent<RawEvent>> {
public:
    /**
     * \brief Construct a new `NextEvent`.
     * \param windows_api the Windows API wrapper
     */
    explicit NextEvent(WindowsApi& windows_api);

    [[nodiscard]] boost::asio::awaitable<evget::Result<evget::InputEvent<RawEvent>>> Next() const override;

private:
    std::reference_wrapper<WindowsApi> windows_api_;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_NEXT_EVENT_H
