#ifndef EVGETX11_EVENTTRANSFORMERX11_H
#define EVGETX11_EVENTTRANSFORMERX11_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <boost/numeric/conversion/cast.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event_transformer.h"
#include "evgetx11/event_switch.h"
#include "evgetx11/event_switch_pointer_key.h"
#include "evgetx11/event_switch_touch.h"
#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

namespace evgetx11 {
template <typename... Switches>
class EventTransformer : public evget::EventTransformer<InputEvent> {
public:
    EventTransformer(X11Api& x_wrapper, EventSwitch x_event_switch, Switches... switches);
    evget::Data TransformEvent(InputEvent event) override;

private:
    std::optional<std::chrono::microseconds> GetInterval(Time time);
    void RefreshDevices();

    std::reference_wrapper<X11Api> x_wrapper_;
    EventSwitch x_event_switch_;
    std::optional<Time> previous_{std::nullopt};
    std::optional<Time> previous_from_event_{std::nullopt};

    std::unordered_map<int, evget::DeviceType> devices_;
    std::unordered_map<int, std::string> id_to_name_;

    std::tuple<Switches...> switches_;
    std::optional<int> pointer_id_;
};

class EventTransformerBuilder {
public:
    EventTransformerBuilder& PointerKey(X11Api& x_wrapper);
    EventTransformerBuilder& Touch();
    std::unique_ptr<evget::EventTransformer<InputEvent>> Build(X11Api& x_wrapper) &&;

private:
    std::optional<EventSwitchPointerKey> pointer_key_;
    std::optional<EventSwitchTouch> touch_;
};

template <typename... Switches>
evgetx11::EventTransformer<Switches...>::EventTransformer(
    X11Api& x_wrapper,
    EventSwitch x_event_switch,
    Switches... switches
)
    : evget::EventTransformer<InputEvent>{},
      x_wrapper_{x_wrapper},
      x_event_switch_{std::move(x_event_switch)},
      switches_{std::move(switches)...} {
    RefreshDevices();
}

template <typename... Switches>
void evgetx11::EventTransformer<Switches...>::RefreshDevices() {
    int n_devices = 0;
    int xi2_n_devices = 0;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = x_wrapper_.get().ListInputDevices(n_devices);
    auto xi2_info = x_wrapper_.get().QueryDevice(xi2_n_devices);

    if (n_devices != xi2_n_devices) {
        spdlog::warn("Devices with ids greater than 127 are not supported.");
    }

    std::map<int, std::reference_wrapper<const XIDeviceInfo>> xi2_devices{};
    for (int i = 0; i < xi2_n_devices; i++) {
        xi2_devices.emplace(xi2_info[i].deviceid, xi2_info[i]);
    }

    for (int i = 0; i < n_devices; i++) {
        const auto& device = info[i];
        int device_id = boost::numeric_cast<int>(device.id);

        if (device.use == IsXPointer) {
            this->pointer_id_ = device_id;
        }

        const auto& xi2_device = xi2_devices.at(device_id).get();

        if ((xi2_device.enabled != False) && device.type != None) {
            auto type = x_wrapper_.get().AtomName(device.type);
            evget::DeviceType device_type{evget::DeviceType::kUnknown};

            if (strcmp(type.get(), XI_MOUSE) == 0) {
                device_type = evget::DeviceType::kMouse;
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                device_type = evget::DeviceType::kKeyboard;
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                device_type = evget::DeviceType::kTouchscreen;
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                device_type = evget::DeviceType::kTouchpad;
            }

            devices_.emplace(device_id, device_type);
            id_to_name_.emplace(device_id, device.name);

            // Iterate through switches and refresh devices.
            std::apply(
                [this, &device_id, &device_type, &device, &xi2_devices](auto&&... event_switches) {
                    ((event_switches.RefreshDevices(
                         device_id,
                         pointer_id_,
                         device_type,
                         device.name,
                         xi2_devices.at(device_id).get(),
                         x_event_switch_
                     )),
                     ...);
                },
                switches_
            );
        }
    }
}

template <typename... Switches>
std::optional<std::chrono::microseconds> evgetx11::EventTransformer<Switches...>::GetInterval(Time time) {
    if (!previous_.has_value() || time < *previous_) {
        previous_ = time;
        return std::nullopt;
    }

    std::chrono::microseconds interval{time - *previous_};
    previous_ = time;

    return interval;
}

template <typename... Switches>
evget::Data evgetx11::EventTransformer<Switches...>::TransformEvent(InputEvent event) {
    evget::Data data{};
    if (event.HasData()) {
        auto type = event.GetEventType();

        if (type == XI_DeviceChanged || type == XI_HierarchyChanged) {
            RefreshDevices();
            return data;
        }

        // Iterate through switches until the first one returns true.
        std::apply(
            [&event, &data, this](auto&&... event_switches) {
                ((event_switches.SwitchOnEvent(
                     event,
                     data,
                     x_event_switch_,
                     [this, &event](Time time) {
                         auto interval = GetInterval(time);
                         if (interval.has_value()) {
                             spdlog::trace(
                                 "interval {} with event type {}",
                                 interval.value().count(),
                                 event.GetEventType()
                             );
                         }

                         return interval;
                     }
                 )) ||
                 ...);
            },
            switches_
        );
    }
    return data;
}
}  // namespace evgetx11

#endif
