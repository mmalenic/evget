#include "evgetx11/event_handler.h"

#include <boost/asio/awaitable.hpp>

#include <memory>
#include <utility>

#include "evget/error.h"
#include "evget/event_loop.h"
#include "evget/event_transformer.h"
#include "evget/storage/store.h"
#include "evgetx11/event_loop.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"
#include "evgetx11/x11_api.h"

evgetx11::EventHandler::EventHandler(
    evget::Store& storage,
    std::unique_ptr<evget::EventLoop<InputEvent>> event_loop,
    std::unique_ptr<evget::EventTransformer<InputEvent>> transformer
)
    : transformer_{std::move(transformer)},
      event_loop_{std::move(event_loop)},
      handler_{storage, *transformer_, *event_loop_} {}

boost::asio::awaitable<evget::Result<void>> evgetx11::EventHandler::Notify(InputEvent event) {
    co_return co_await handler_.Notify(std::move(event));
}

boost::asio::awaitable<evget::Result<void>> evgetx11::EventHandler::Start() {
    // NOLINTBEGIN(clang-analyzer-core.CallAndMessage)
    co_return co_await handler_.Start();
    // NOLINTEND(clang-analyzer-core.CallAndMessage)
}

void evgetx11::EventHandler::Stop() {
    handler_.Stop();
}

evgetx11::EventHandlerBuilder& evgetx11::EventHandlerBuilder::PointerKey(X11Api& x_wrapper) {
    builder_ = builder_.PointerKey(x_wrapper);
    return *this;
}

evgetx11::EventHandlerBuilder& evgetx11::EventHandlerBuilder::Touch() {
    builder_ = builder_.Touch();
    return *this;
}

evget::Result<evgetx11::EventHandler> evgetx11::EventHandlerBuilder::Build(evget::Store& storage, X11Api& x11_api) && {
    auto transformer = std::move(builder_).Build(x11_api);
    auto input_handler = InputHandlerBuilder::Build(x11_api);
    if (!input_handler.has_value()) {
        return evget::Err{input_handler.error()};
    }
    auto event_loop = EventLoopBuilder::Build(*input_handler);

    return evget::Result<EventHandler>(std::in_place, storage, std::move(event_loop), std::move(transformer));
}
