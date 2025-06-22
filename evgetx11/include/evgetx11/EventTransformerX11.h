// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
#define EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H

#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xlib.h>
#include <boost/numeric/conversion/cast.hpp>
#include <spdlog/spdlog.h>

#include <concepts>
#include <map>
#include <set>
#include <unordered_map>

#include "XEventSwitch.h"
#include "XEventSwitchPointerKey.h"
#include "XEventSwitchTouch.h"
#include "XInputHandler.h"
#include "XWrapper.h"
#include "evgetcore/Event/MouseScroll.h"
#include "evgetcore/EventTransformer.h"


namespace EvgetX11 {
template <typename... Switches>
class EventTransformerX11 : public EvgetCore::EventTransformer<XInputEvent> {
public:
    explicit EventTransformerX11(XWrapper& xWrapper, XEventSwitch xEventSwitch, Switches... switches);
    EvgetCore::Event::Data transformEvent(XInputEvent event) override;

    static EventTransformerX11 build(XWrapper& xWrapper);
private:
    std::optional<std::chrono::microseconds> getInterval(Time time);
    void refreshDevices();

    std::reference_wrapper<XWrapper> xWrapper;
    XEventSwitch xEventSwitch;
    std::optional<Time> previous{std::nullopt};
    std::optional<Time> previousFromEvent{std::nullopt};

    std::unordered_map<int, EvgetCore::Event::Device> devices{};
    std::unordered_map<int, std::string> idToName{};

    std::tuple<Switches...> switches;
    std::optional<int> pointer_id{};
};

template <typename... Switches>
EvgetX11::EventTransformerX11<Switches...>::EventTransformerX11(XWrapper& xWrapper, XEventSwitch xEventSwitch, Switches... switches)
    : xWrapper{xWrapper}, xEventSwitch{xEventSwitch}, switches{switches...} {
    refreshDevices();
}

template <typename... Switches>
void EvgetX11::EventTransformerX11<Switches...>::refreshDevices() {
    int nDevices = 0;
    int xi2NDevices = 0;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = xWrapper.get().listInputDevices(nDevices);
    auto xi2Info = xWrapper.get().queryDevice(xi2NDevices);

    if (nDevices != xi2NDevices) {
        spdlog::warn(
            "Devices with ids greater than 127 are not supported."
        );
    }

    std::map<int, std::reference_wrapper<const XIDeviceInfo>> xi2Devices{};
    for (int i = 0; i < xi2NDevices; i++) {
        xi2Devices.emplace(xi2Info[i].deviceid, xi2Info[i]);
    }

    for (int i = 0; i < nDevices; i++) {
        const auto& device = info[i];
        int id = boost::numeric_cast<int>(device.id);

        if (device.use == IsXPointer) {
            this->pointer_id = id;
        }

        const auto& xi2Device = xi2Devices.at(id).get();

        if (xi2Device.enabled && device.type != None) {
            auto type = xWrapper.get().atomName(device.type);
            EvgetCore::Event::Device deviceType;

            if (strcmp(type.get(), XI_MOUSE) == 0) {
                deviceType = EvgetCore::Event::Device::Mouse;
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                deviceType = EvgetCore::Event::Device::Keyboard;
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                deviceType = EvgetCore::Event::Device::Touchscreen;
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                deviceType = EvgetCore::Event::Device::Touchpad;
            } else {
                deviceType = EvgetCore::Event::Device::Unknown;
            }

            devices.emplace(id, deviceType);
            idToName.emplace(id, device.name);

            // Iterate through switches and refresh devices.
            std::apply(
                [this, &id, &deviceType, &device, &xi2Devices](auto&&... eventSwitches) {
                    ((eventSwitches.refreshDevices(id, pointer_id, deviceType, device.name, xi2Devices.at(id).get(), xEventSwitch)), ...);
                },
                switches
            );
        }
    }
}

template <typename... Switches>
std::optional<std::chrono::microseconds> EvgetX11::EventTransformerX11<Switches...>::getInterval(Time time) {
    if (!previous.has_value() || time < *previous) {
        previous = time;
        return std::nullopt;
    }

    std::chrono::microseconds interval{time - *previous};
    previous = time;

    return interval;
}

template <typename... Switches>
EvgetCore::Event::Data EvgetX11::EventTransformerX11<Switches...>::transformEvent(XInputEvent event) {
    EvgetCore::Event::Data data{};
    if (event.hasData()) {
        auto type = event.getEventType();

        if (type == XI_DeviceChanged || type == XI_HierarchyChanged) {
            refreshDevices();
            return data;
        }

        // Iterate through switches until the first one returns true.
        std::apply(
            [&event, &data, this](auto&&... eventSwitches) {
                ((eventSwitches.switchOnEvent(event, data, xEventSwitch, [this, &event](Time time) {
                     auto interval = getInterval(time);
                     if (interval.has_value()) {
                         spdlog::trace(
                             "interval {} with event type {}",
                             interval.value().count(),
                             event.getEventType()
                         );
                     }

                     return interval;
                 })) || ...);
            },
            switches
        );
    }
    return data;
}

template<typename ... Switches>
EventTransformerX11<Switches...> EventTransformerX11<Switches...>::build(XWrapper& xWrapper) {
    EvgetX11::XEventSwitch xEventSwitch{xWrapper};
    return EventTransformerX11{xWrapper, xEventSwitch};
}

    template<>
EventTransformerX11<XEventSwitchPointerKey, XEventSwitchTouch> EventTransformerX11<XEventSwitchPointerKey, XEventSwitchTouch>::build(XWrapper& xWrapper) {
    EvgetX11::XEventSwitch xEventSwitch{xWrapper};
    EvgetX11::XEventSwitchPointerKey xEventSwitchPointer{xWrapper};
    EvgetX11::XEventSwitchTouch xEventSwitchTouch{};

    return EventTransformerX11{xWrapper, xEventSwitch, xEventSwitchPointer, xEventSwitchTouch};
}
}  // namespace EvgetX11

#endif  // EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
