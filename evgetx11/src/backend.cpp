#include "evgetx11/backend.h"

#include <X11/Xlib.h>

#include <expected>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/storage/store.h"
#include "evgetx11/event_transformer.h"
#include "evgetx11/input_event.h"
#include "evgetx11/input_handler.h"

evgetx11::Backend::Backend(std::unique_ptr<Display, decltype(&XCloseDisplay)> display)
    : display_(std::move(display)), api_(*this->display_) {}

evget::Result<std::unique_ptr<evgetx11::Backend>>
evgetx11::Backend::Create(evget::Store& storage, const std::optional<std::string>& display) {
    const char* display_name = nullptr;
    if (display.has_value()) {
        display_name = display->c_str();
    }

    std::unique_ptr<Display, decltype(&XCloseDisplay)> display_ptr{XOpenDisplay(display_name), XCloseDisplay};
    if (display_ptr == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = std::format("unable to open X11 display '{}'", display_name)}
        };
    }

    auto backend = std::unique_ptr<Backend>(new Backend(std::move(display_ptr)));

    backend->transformer_ = std::move(EventTransformerBuilder{}.PointerKey(backend->api_).Touch()).Build(backend->api_);

    auto next_event = InputHandlerBuilder::Build(backend->api_);
    if (!next_event.has_value()) {
        return std::unexpected(next_event.error());
    }
    backend->next_event_ = std::move(*next_event);

    backend->handler_.emplace(storage, *backend->transformer_, *backend->next_event_);
    return backend;
}

evget::Result<std::reference_wrapper<evget::EventHandler<evgetx11::InputEvent>>> evgetx11::Backend::Handler() {
    if (!handler_.has_value()) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = "`Handler` called before `Create` completed"}
        };
    }
    return std::ref(*handler_);
}
