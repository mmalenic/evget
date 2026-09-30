#include "evgetx11/event_transformer.h"

#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XIproto.h>

#include <algorithm>
#include <format>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "evget/event/device_type.h"
#include "evget/event_transformer.h"
#include "evgetx11/input_event.h"
#include "evgetx11/log_once.h"
#include "evgetx11/x11.h"

evget::DeviceType evgetx11::InjectedDeviceType(int use) {
    switch (use) {
        case XISlaveKeyboard:
            return evget::DeviceType::kKeyboard;
        case XISlavePointer:
            return evget::DeviceType::kMouse;
        default:
            return evget::DeviceType::kUnknown;
    }
}

evget::DeviceType evgetx11::Xi1DeviceType(const char* type_name) {
    if (type_name == nullptr) {
        return evget::DeviceType::kUnknown;
    }
    const std::string_view type{type_name};
    if (type == XI_MOUSE) {
        return evget::DeviceType::kMouse;
    }
    if (type == XI_KEYBOARD) {
        return evget::DeviceType::kKeyboard;
    }
    if (type == XI_TOUCHPAD) {
        return evget::DeviceType::kTouchpad;
    }
    if (type == XI_TOUCHSCREEN) {
        return evget::DeviceType::kTouchscreen;
    }
    // See
    // https://gitlab.freedesktop.org/xorg/driver/xf86-input-libinput/-/blob/ac862672e4d04e78f2b647af9d3d14544454e4b9/src/xf86libinput.c#L3805-3846
    if (type == XI_TABLET || type == "PAD" || type == "STYLUS" || type == "ERASER" || type == "CURSOR") {
        return evget::DeviceType::kTablet;
    }
    return evget::DeviceType::kUnknown;
}

void evgetx11::WarnUnlistedDevices(
    std::span<const XDeviceInfo> xi1_devices,
    std::span<const XIDeviceInfo> xi2_devices,
    LogOnce& log
) {
    for (const auto& device : xi2_devices) {
        const auto listed = std::ranges::any_of(xi1_devices, [&device](const XDeviceInfo& xi1_device) {
            return std::cmp_equal(xi1_device.id, device.deviceid);
        });
        if (!listed && device.deviceid > DEVICE_BITS) {
            log.Warn(
                std::format(
                    "device '{}' has ID {}, above the {} that XInput 1 can list, so its events are not captured",
                    device.name,
                    device.deviceid,
                    DEVICE_BITS
                )
            );
        }
    }
}

evgetx11::EventTransformerBuilder& evgetx11::EventTransformerBuilder::PointerKey(X11Api& x_wrapper) {
    pointer_key_ = EventSwitchPointerKey{x_wrapper};
    return *this;
}

evgetx11::EventTransformerBuilder& evgetx11::EventTransformerBuilder::Touch() {
    touch_ = EventSwitchTouch{};
    return *this;
}

std::unique_ptr<evget::EventTransformer<evgetx11::InputEvent>> evgetx11::EventTransformerBuilder::Build(
    X11Api& x_wrapper
) && {
    if (pointer_key_.has_value() && touch_.has_value()) {
        return std::make_unique<EventTransformer<EventSwitchPointerKey, EventSwitchTouch>>(
            x_wrapper,
            EventSwitch{x_wrapper},
            std::move(*pointer_key_),
            *touch_
        );
    }
    if (pointer_key_.has_value()) {
        return std::make_unique<EventTransformer<EventSwitchPointerKey>>(
            x_wrapper,
            EventSwitch{x_wrapper},
            std::move(*pointer_key_)
        );
    }
    if (touch_.has_value()) {
        return std::make_unique<EventTransformer<EventSwitchTouch>>(x_wrapper, EventSwitch{x_wrapper}, *touch_);
    }

    return nullptr;
}
