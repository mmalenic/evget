#include "evgetx11/event_transformer.h"

#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XIproto.h>

#include <algorithm>
#include <format>
#include <memory>
#include <span>
#include <utility>

#include "evget/event_transformer.h"
#include "evgetx11/input_event.h"
#include "evgetx11/log_once.h"
#include "evgetx11/x11.h"

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
