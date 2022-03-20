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

std::vector<std::unique_ptr<EvgetCore::Event::TableData>> EvgetX11::EventTransformerLinux::transformEvent(EvgetX11::XInputEvent event) {
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>> data{};
    if (event.hasData()) {
        auto type = event.getEventType();

        if (type == XI_DeviceChanged) {
            deviceChangedEvent(event, getTime(event), data);
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

void EvgetX11::EventTransformerLinux::deviceChangedEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data) {
    auto changedEvent = event.viewData<XIDeviceChangedEvent>();
    if (devices.contains(changedEvent.deviceid)) {
        std::vector<std::unique_ptr<EvgetCore::Event::AbstractField>> fields{};

        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceName", idToName[changedEvent.deviceid]));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Common::Time>(timestamp));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("XInputTime", std::to_string(changedEvent.time)));
        fields
            .emplace_back(std::make_unique<EvgetCore::Event::Field>("DeviceId", std::to_string(changedEvent.deviceid)));
        fields
            .emplace_back(std::make_unique<EvgetCore::Event::Field>("SourceId", std::to_string(changedEvent.sourceid)));
        fields.emplace_back(std::make_unique<EvgetCore::Event::Field>("Reason", std::to_string(changedEvent.reason)));
        fields.emplace_back(
            std::make_unique<EvgetCore::Event::Field>(
                "ReasonName",
                (changedEvent.reason == XISlaveSwitch) ? "SlaveSwitch" : "DeviceChanged"
            )
        );

        XEventSwitch::addTableData(data, {}, std::make_unique<EvgetCore::Event::Data>("DeviceChangedEvent", std::move(fields)));
    }
}

void EvgetX11::EventTransformerLinux::refreshDevices() {
    int nDevices;
    int xi2NDevices;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>(XListInputDevices(&display.get(), &nDevices),
        XFreeDeviceList);
    auto xi2Info = std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>(XIQueryDevice(&display.get(), XIAllDevices, &xi2NDevices),
        XIFreeDeviceInfo);

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
            auto type = XEventSwitch::getAtomName(display, device.type);
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

EvgetX11::EventTransformerLinux::EventTransformerLinux(Display& display, std::initializer_list<std::reference_wrapper<XEventSwitch>> switches) : display{display}, switches{switches} {
    refreshDevices();
}

std::map<int, std::string> EvgetX11::EventTransformerLinux::typeToName() {
    std::map<int, std::string> map{};

    map.emplace(XI_DeviceChanged, "DeviceChanged");
    map.emplace(XI_KeyPress, "KeyPress");
    map.emplace(XI_KeyRelease, "KeyRelease");
    map.emplace(XI_ButtonPress, "ButtonPress");
    map.emplace(XI_ButtonRelease, "ButtonRelease");
    map.emplace(XI_Motion, "Motion");
    map.emplace(XI_Enter, "Enter");
    map.emplace(XI_Leave, "Leave");
    map.emplace(XI_FocusIn, "FocusIn");
    map.emplace(XI_FocusOut, "FocusOut");
    map.emplace(XI_HierarchyChanged, "HierarchyChanged");
    map.emplace(XI_PropertyEvent, "PropertyEvent");
    map.emplace(XI_RawKeyPress, "RawKeyPress");
    map.emplace(XI_RawKeyRelease, "RawKeyRelease");
    map.emplace(XI_RawButtonPress, "RawButtonPress");
    map.emplace(XI_RawButtonRelease, "RawButtonRelease");
    map.emplace(XI_RawMotion, "RawMotion");
#if defined EVGETX11_HAS_TOUCH_SUPPORT
    map.emplace(XI_TouchBegin, "TouchBegin");
    map.emplace(XI_TouchUpdate, "TouchUpdate");
    map.emplace(XI_TouchEnd, "TouchEnd");
    map.emplace(XI_RawTouchBegin, "RawTouchBegin");
    map.emplace(XI_RawTouchUpdate, "RawTouchUpdate");
    map.emplace(XI_RawTouchEnd, "RawTouchEnd");
#endif
#if defined EVGETX11_HAS_BARRIER_SUPPORT
    map.emplace(XI_BarrierHit, "BarrierHit");
    map.emplace(XI_BarrierLeave, "BarrierLeave");
#endif
#if defined EVGETX11_HAS_GESTURE_SUPPORT
    map.emplace(XI_BarrierHit, "BarrierHit");
    map.emplace(XI_BarrierLeave, "BarrierLeave");
#endif

    return map;
}
