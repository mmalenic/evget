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


#include <spdlog/spdlog.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
#include <xorg/xserver-properties.h>
#include <boost/numeric/conversion/cast.hpp>
#include "evgetx11/EventTransformerLinux.h"
#include "evgetcore/UnsupportedOperationException.h"
#include "evgetcore/Event/MouseClick.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/Field.h"
#include "evgetcore/Event/MouseScroll.h"
#include "evgetcore/Event/MouseMove.h"
#include "evgetx11/XEventSwitch.h"

std::vector<std::unique_ptr<EvgetCore::Event::TableData>> EvgetX11::EventTransformerLinux::transformEvent(XInputEvent event) {
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>> data{};
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

std::chrono::nanoseconds EvgetX11::EventTransformerLinux::getTime(const EvgetX11::XInputEvent& event) {
    if (!start.has_value()) {
        start = event.getTimestamp();
    }
    return event.getTimestamp() - *start;
}

void EvgetX11::EventTransformerLinux::refreshDevices() {
    int nDevices;
    int xi2NDevices;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = xWrapper.get().listInputDevices(nDevices);
    auto xi2Info = xWrapper.get().queryDevice(xi2NDevices);

    if (nDevices != xi2NDevices) {
        spdlog::warn("Devices with ids greater than 127 found. Set the device of these devices manually if their use is required.");
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
            EvgetCore::Event::Common::Device deviceType;

            if (strcmp(type.get(), XI_MOUSE) == 0) {
                deviceType = EvgetCore::Event::Common::Device::Mouse;
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                deviceType = EvgetCore::Event::Common::Device::Keyboard;
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                deviceType = EvgetCore::Event::Common::Device::Touchscreen;
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                deviceType = EvgetCore::Event::Common::Device::Touchpad;
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

EvgetX11::EventTransformerLinux::EventTransformerLinux(XWrapper& xWrapper, std::initializer_list<std::reference_wrapper<XEventSwitch>> switches) : xWrapper{xWrapper}, switches{switches} {
    refreshDevices();
}