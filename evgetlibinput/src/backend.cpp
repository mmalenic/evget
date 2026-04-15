#include "evgetlibinput/backend.h"

#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "evget/error.h"
#include "evget/event_handler.h"
#include "evget/input_event.h"
#include "evget/storage/store.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/libinput.h"
#include "evgetlibinput/xkbcommon.h"

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

evget::Result<std::unique_ptr<evgetlibinput::Backend>> evgetlibinput::Backend::Create(
    std::optional<std::pair<std::uint32_t, std::uint32_t>> dimensions,
    evget::Store& storage,
    const std::optional<std::string>& seat
) {
    ScreenDimensions resolved_dimensions{};
    if (dimensions) {
        resolved_dimensions = {.width = dimensions->first, .height = dimensions->second};
    } else {
        auto drm = DrmOutput::New();
        if (!drm.has_value()) {
            return std::unexpected(drm.error());
        }
        resolved_dimensions = drm->GetDimensions();
    }

    auto libinput = LibInput::New(seat);
    if (!libinput.has_value()) {
        return std::unexpected(libinput.error());
    }

    auto xkb = XkbCommon::New();
    if (!xkb.has_value()) {
        return std::unexpected(xkb.error());
    }

    return std::unique_ptr<Backend>(new Backend(std::move(*libinput), std::move(*xkb), resolved_dimensions, storage));
}

evget::EventHandler<evget::InputEvent<evgetlibinput::LibInputEvent>>& evgetlibinput::Backend::Handler() {
    return handler_;
}
