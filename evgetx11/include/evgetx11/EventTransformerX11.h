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

#include <unordered_map>
#include <X11/extensions/XInput2.h>
#include <set>
#include <map>
#include <concepts>
#include <spdlog/spdlog.h>
#include <boost/numeric/conversion/cast.hpp>
#include "XInputHandler.h"
#include "evgetcore/EventTransformer.h"
#include "evgetcore/Util.h"
#include "evgetcore/Event/MouseScroll.h"
#include "evgetx11/XDeviceRefresh.h"
#include "XWrapper.h"
#include "evgetcore/UnsupportedOperationException.h"

namespace EvgetX11 {
    template<XEventSwitch... T>
    class EventTransformerX11 : EvgetCore::EventTransformer<XInputEvent> {
    public:
        EventTransformerX11(XWrapper& xWrapper, T&... switches);
        std::vector<EvgetCore::Event::Data> transformEvent(XInputEvent event) override;

    private:
        std::chrono::nanoseconds getTime(const XInputEvent& event);
        void refreshDevices();

        std::reference_wrapper<XWrapper> xWrapper;
        std::optional<XInputEvent::Timestamp> start{std::nullopt};

        std::unordered_map<int, EvgetCore::Event::Device> devices{};
        std::unordered_map<int, std::string> idToName{};

        std::tuple<T&...> switches;
    };

    template<XEventSwitch... T>
    EvgetX11::EventTransformerX11<T...>::EventTransformerX11(EvgetX11::XWrapper &xWrapper, T&... switches):
    xWrapper{xWrapper}, switches{std::make_tuple(std::forward<T>(switches)...)} {
        refreshDevices();
    }

    template<XEventSwitch... T>
    void EvgetX11::EventTransformerX11<T...>::refreshDevices() {
        int nDevices;
        int xi2NDevices;
        // See caveats about mixing XI1 calls with XI2 code:
        // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
        // This should capture all devices with ids in the range 0-128.
        auto info = xWrapper.get().listInputDevices(nDevices);
        auto xi2Info = xWrapper.get().queryDevice(xi2NDevices);

        if (nDevices != xi2NDevices) {
            spdlog::warn("Devices with ids greater than 127 found. Set the id of these devices manually if their use is required.");
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

            if (xi2Device.enabled && device.type != None && (device.use == IsXExtensionPointer || device.use == IsXExtensionKeyboard || device.use == IsXExtensionDevice)) {
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
                    spdlog::info("Unsupported class type '{}' from XDeviceInfo for device '{}' with id {}.", type.get(), device.name, device.id);
                    continue;
                }

                devices.emplace(id, deviceType);
                idToName.emplace(id, device.name);

                for (const auto& eventSwitch : switches) {
                    eventSwitch.get().refreshDevices(id, deviceType, device.name, xi2Devices.at(id).get());
                }
            }
        }
    }

    template<XEventSwitch... T>
    std::chrono::nanoseconds EvgetX11::EventTransformerX11<T...>::getTime(const EvgetX11::XInputEvent& event) {
        if (!start.has_value()) {
            start = event.getTimestamp();
        }
        return event.getTimestamp() - *start;
    }

    template<XEventSwitch... T>
    std::vector<EvgetCore::Event::Data> EvgetX11::EventTransformerX11<T...>::transformEvent(XInputEvent event) {
        std::vector<EvgetCore::Event::Data> data{};
        if (event.hasData()) {
            auto type = event.getEventType();

            if (type == XI_DeviceChanged || type == XI_HierarchyChanged) {
                refreshDevices();
                return data;
            }

            for (const auto& eventSwitches : switches) {
                if (eventSwitches.get().switchOnEvent(event, getTime(event), data)) {
                    return data;
                }
            }
        }
        return data;
    }
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_EVENTTRANSFORMERLINUX_H
