#include "evgetx11/XInputHandler.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <evget/error.h>
#include <spdlog/spdlog.h>

#include <array>
#include <format>

#include "evgetx11/XInputEvent.h"
#include "evgetx11/XWrapper.h"
#include "evgetx11/XWrapperX11.h"

evgetx11::XInputHandler::XInputHandler(XWrapper& x_wrapper) : x_wrapper_{x_wrapper} {}

evget::Result<void> evgetx11::XInputHandler::AnnounceVersion(XWrapper& x_wrapper) {
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

void evgetx11::XInputHandler::SetMask(XWrapper& x_wrapper) {
    XIEventMask mask{};
    mask.deviceid = XIAllMasterDevices;

    std::array<unsigned char, XI_LASTEVENT> event_mask{};
    XWrapperX11::SetMask(
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

evget::Result<evgetx11::XInputHandler> evgetx11::XInputHandler::Build(XWrapper& x_wrapper) {
    return AnnounceVersion(x_wrapper).transform([&x_wrapper] {
        SetMask(x_wrapper);
        return XInputHandler{x_wrapper};
    });
}

evgetx11::XInputEvent evgetx11::XInputHandler::GetEvent() const {
    return XInputEvent::NextEvent(x_wrapper_.get());
}
