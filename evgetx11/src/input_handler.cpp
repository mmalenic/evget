#include "evgetx11/input_handler.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <boost/asio/awaitable.hpp>
#include <evget/error.h>
#include <spdlog/spdlog.h>

#include <array>
#include <format>

#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

evgetx11::InputHandler::InputHandler(X11Api& x_wrapper) : x_wrapper_{x_wrapper} {}

evget::Result<void> evgetx11::InputHandlerBuilder::AnnounceVersion(X11Api& x_wrapper) {
    int major = kVersionMajor;
    int minor = kVersionMinor;

    Status status = x_wrapper.QueryVersion(major, minor);
    if (status == Success) {
        spdlog::info("XI2 is supported with version {}.{}", major, minor);
        return {};
    }

    return evget::Err{
        {.error_type = evget::ErrorType::kEventHandlerError,
         .message = std::format("XI2 is not supported, only version {}.{} is available.", major, minor)}
    };
}

void evgetx11::InputHandlerBuilder::SetMask(X11Api& x_wrapper) {
    XIEventMask mask{};
    mask.deviceid = XIAllMasterDevices;

    std::array<unsigned char, XI_LASTEVENT> event_mask{};
    X11ApiImpl::SetMask(
        event_mask.data(),
        {
            XI_RawButtonPress,
            XI_RawButtonRelease,
            XI_RawKeyPress,
            XI_RawKeyRelease,
            XI_RawMotion,
            XI_DeviceChanged,
            XI_RawTouchBegin,
            XI_RawTouchEnd,
            XI_RawTouchUpdate,
        }
    );

    mask.mask_len = sizeof(event_mask);
    mask.mask = event_mask.data();

    x_wrapper.SelectEvents(mask);
}

boost::asio::awaitable<evget::Result<evgetx11::InputEvent>> evgetx11::InputHandler::Next() const {
    co_return InputEvent::NextEvent(x_wrapper_.get());
}

evget::Result<evgetx11::InputHandler> evgetx11::InputHandlerBuilder::Build(X11Api& x_wrapper) {
    return AnnounceVersion(x_wrapper).transform([&x_wrapper] {
        SetMask(x_wrapper);
        return InputHandler{x_wrapper};
    });
}
