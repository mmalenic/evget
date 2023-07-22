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

#include <X11/extensions/XInput2.h>
#include <boost/numeric/conversion/cast.hpp>
#include <spdlog/spdlog.h>

#include <concepts>
#include <map>
#include <set>
#include <unordered_map>

#include "XInputHandler.h"
#include "XWrapper.h"
#include "evgetcore/Event/MouseScroll.h"
#include "evgetcore/EventTransformer.h"
#include "evgetcore/UnsupportedOperationException.h"
#include "evgetcore/Util.h"

namespace EvgetX11 {
template <XWrapper XWrapper, typename... Switches>
class EventTransformerX11 : public EvgetCore::EventTransformer<XInputEvent> {
public:
    explicit EventTransformerX11(XWrapper& xWrapper, Switches&... switches);
    EvgetCore::Event::Data transformEvent(XInputEvent event) override;

private:
    std::optional<std::chrono::microseconds> getInterval(Time time);
    void refreshDevices();

    std::reference_wrapper<XWrapper> xWrapper;
    std::optional<Time> previous{std::nullopt};
    std::optional<Time> previousFromEvent{std::nullopt};

    std::unordered_map<int, EvgetCore::Event::Device> devices{};
    std::unordered_map<int, std::string> idToName{};

    std::tuple<Switches&...> switches;
};

template <XWrapper XWrapper, typename... Switches>
EvgetX11::EventTransformerX11<XWrapper, Switches...>::EventTransformerX11(XWrapper& xWrapper, Switches&... switches)
    : xWrapper{xWrapper}, switches{switches...} {
    refreshDevices();
}

template <XWrapper XWrapper, typename... Switches>
void EvgetX11::EventTransformerX11<XWrapper, Switches...>::refreshDevices() {
    int nDevices;
    int xi2NDevices;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = xWrapper.get().listInputDevices(nDevices);
    auto xi2Info = xWrapper.get().queryDevice(xi2NDevices);

    if (nDevices != xi2NDevices) {
        spdlog::warn(
            "Devices with ids greater than 127 found. Set the id of these devices manually if their use is required."
        );
    }

    std::map<int, std::reference_wrapper<const XIDeviceInfo>> xi2Devices{};
    for (int i = 0; i < xi2NDevices; i++) {
        xi2Devices.emplace(xi2Info[i].deviceid, xi2Info[i]);
    }

    for (int i = 0; i < nDevices; i++) {
        const auto& device = info[i];
        int id = boost::numeric_cast<int>(device.id);

        if (!xi2Devices.contains(id)) {
            throw EvgetCore::UnsupportedOperationException{"Device id from XDeviceInfo not found in XIDeviceInfo."};
        }
        const auto& xi2Device = xi2Devices.at(id).get();

        if (xi2Device.enabled && device.type != None &&
            (device.use == IsXExtensionPointer || device.use == IsXExtensionKeyboard || device.use == IsXExtensionDevice
            )) {
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
                spdlog::info(
                    "Unsupported class type '{}' from XDeviceInfo for device '{}' with id {}.",
                    type.get(),
                    device.name,
                    device.id
                );
                continue;
            }

            devices.emplace(id, deviceType);
            idToName.emplace(id, device.name);

            // Iterate through switches and refresh devices.
            std::apply(
                [&id, &deviceType, &device, &xi2Devices](auto&&... eventSwitches) {
                    ((eventSwitches.refreshDevices(id, deviceType, device.name, xi2Devices.at(id).get())), ...);
                },
                switches
            );
        }
    }
}

template <XWrapper XWrapper, typename... Switches>
std::optional<std::chrono::microseconds> EvgetX11::EventTransformerX11<XWrapper, Switches...>::getInterval(Time time) {
    if (!previous.has_value() || time < *previous) {
        previous = time;
        return std::nullopt;
    }

    std::chrono::microseconds interval{time - *previous};
    previous = time;

    return interval;
}

template <XWrapper XWrapper, typename... Switches>
EvgetCore::Event::Data EvgetX11::EventTransformerX11<XWrapper, Switches...>::transformEvent(XInputEvent event) {
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
                ((eventSwitches.switchOnEvent(event, data, [this, &event](Time time) {
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
}  // namespace EvgetX11

#endif  // EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
