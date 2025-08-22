#ifndef EVGETX11_EVENTLOOPX11_H
#define EVGETX11_EVENTLOOPX11_H

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <functional>
#include <optional>

#include "evget/error.h"
#include "evget/event_listener.h"
#include "evget/event_loop.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XInputHandler.h"

namespace evgetx11 {

/**
 * Class represents processing evgetx11 system events.
 */
class EventLoopX11 : public evget::EventLoop<XInputEvent> {
public:
    /**
     * Create the system events.
     */
    explicit EventLoopX11(XInputHandler x_input_handler);

    evget::Result<void> Notify(XInputEvent event) override;
    void RegisterEventListener(evget::EventListener<XInputEvent>& event_listener) override;
    void Stop() override;
    boost::asio::awaitable<evget::Result<void>> Start() override;
    boost::asio::awaitable<bool> IsStopped();

private:
    std::optional<std::reference_wrapper<evget::EventListener<XInputEvent>>> event_listener_;
    XInputHandler handler_;

    std::atomic<bool> stopped_{false};
};
}  // namespace evgetx11

#endif
