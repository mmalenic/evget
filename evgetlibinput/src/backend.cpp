#include "evgetlibinput/backend.h"

#include <expected>
#include <memory>
#include <utility>

evgetlibinput::Backend::Backend(
    std::unique_ptr<LibInputApi> libinput,
    XkbCommon xkb,
    ScreenDimensions dimensions,
    evget::Store& storage
)
    : libinput_(std::move(libinput)),
      xkb_(std::move(xkb)),
      transformer_(*this->libinput_, this->xkb_, dimensions),
      next_event_(*this->libinput_),
      handler_(storage, transformer_, next_event_) {}

evget::Result<std::unique_ptr<evgetlibinput::Backend>>
evgetlibinput::Backend::Create(ScreenDimensions dimensions, evget::Store& storage) {
    auto libinput = LibInput::New();
    if (!libinput.has_value()) {
        return std::unexpected(libinput.error());
    }

    auto xkb = XkbCommon::New();
    if (!xkb.has_value()) {
        return std::unexpected(xkb.error());
    }

    return std::unique_ptr<Backend>(new Backend(std::move(*libinput), std::move(*xkb), dimensions, storage));
}

evget::EventHandler<evget::InputEvent<evgetlibinput::LibInputEvent>>& evgetlibinput::Backend::Handler() {
    return handler_;
}
