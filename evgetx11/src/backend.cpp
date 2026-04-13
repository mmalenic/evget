#include "evgetx11/backend.h"

#include <X11/Xlib.h>

#include <expected>
#include <memory>
#include <utility>

#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/storage/store.h"
#include "evgetx11/event_transformer.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"

evgetx11::Backend::Backend(std::unique_ptr<Display, decltype(&XCloseDisplay)> display)
    : display_(std::move(display)), api_(*this->display_) {}

evget::Result<std::unique_ptr<evgetx11::Backend>> evgetx11::Backend::Create(evget::Store& storage) {
    auto backend = std::unique_ptr<Backend>(new Backend({XOpenDisplay(nullptr), XCloseDisplay}));

    backend->transformer_ = std::move(EventTransformerBuilder{}.PointerKey(backend->api_).Touch()).Build(backend->api_);

    auto next_event = InputHandlerBuilder::Build(backend->api_);
    if (!next_event.has_value()) {
        return std::unexpected(next_event.error());
    }
    backend->next_event_ = std::move(*next_event);

    backend->handler_.emplace(storage, *backend->transformer_, *backend->next_event_);
    return backend;
}

evget::EventHandler<evgetx11::InputEvent>& evgetx11::Backend::Handler() {
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    return *handler_;
}
