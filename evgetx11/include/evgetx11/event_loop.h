#ifndef EVGETX11_EVENTLOOPX11_H
#define EVGETX11_EVENTLOOPX11_H

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <optional>

#include "evget/error.h"
#include "evget/event_listener.h"
#include "evget/event_loop.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"

namespace evgetx11 {

/**
 * Class represents processing evgetx11 system events.
 */
class EventLoop : public evget::EventLoop<InputEvent> {
public:
    /**
     * Create the system events.
     */
    explicit EventLoop(InputHandler x_input_handler);

    boost::asio::awaitable<evget::Result<void>> Notify(InputEvent event) override;
    void RegisterEventListener(evget::EventListener<InputEvent>& event_listener) override;
    void Stop() override;
    boost::asio::awaitable<evget::Result<void>> Start() override;
    boost::asio::awaitable<bool> IsStopped();

private:
    std::optional<std::reference_wrapper<evget::EventListener<InputEvent>>> event_listener_;
    InputHandler handler_;

    bool stopped_{false};
};

class EventLoopBuilder {
public:
    static std::unique_ptr<EventLoop> Build(InputHandler input_handler);
};
} // namespace evgetx11

#endif
