#ifndef EVGETX11_EVENT_HANDLER_H
#define EVGETX11_EVENT_HANDLER_H

#include "event_transformer.h"
#include "evget/event_handler.h"
#include "evget/event_listener.h"

namespace evgetx11 {
class EventHandler : public evget::EventListener<InputEvent> {
public:
    EventHandler(
        evget::Store& storage,
        std::unique_ptr<evget::EventLoop<InputEvent>> event_loop,
        std::unique_ptr<evget::EventTransformer<InputEvent>> transformer
    );

    boost::asio::awaitable<evget::Result<void>> Notify(InputEvent event) override;
    boost::asio::awaitable<evget::Result<void>> Start() override;
    void Stop();

private:
    std::unique_ptr<evget::EventTransformer<InputEvent>> transformer_;
    std::unique_ptr<evget::EventLoop<InputEvent>> event_loop_;
    evget::EventHandler<InputEvent> handler_;
};

class EventHandlerBuilder {
public:
    EventHandlerBuilder& PointerKey(X11Api& x_wrapper);
    EventHandlerBuilder& Touch();
    evget::Result<EventHandler> Build(evget::Store& storage, X11Api& x11_api) &&;

private:
    EventTransformerBuilder builder_;
};
} // namespace evgetx11

#endif
