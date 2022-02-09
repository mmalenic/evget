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
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "evget/EventTransformerLinux.h"
#include "evget/UnsupportedOperationException.h"
#include "evget/Event/MouseClick.h"

namespace algorithm = boost::algorithm;

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::transformEvent(evget::XInputEvent event) {
    const auto& deviceEvent = event.viewData<XIDeviceEvent>();

    switch (deviceEvent.evtype) {
    case XI_ButtonPress:
        break;
    case XI_KeyPress:
        break;
    case XI_KeyRelease:
        break;
    case XI_Motion:
        break;
#if defined XI_TouchBegin && defined XI_TouchUpdate && defined XI_TouchEnd
    case XI_TouchBegin:
        break;
    case XI_TouchUpdate:
        break;
    case XI_TouchEnd:
        break;
#endif
    case XI_HierarchyChanged:
    case XI_DeviceChanged:
        refreshDeviceIds();
        break;
    default:
        spdlog::info("Unsupported event with type '{}' from device with id '{}' passed to event transformer.", deviceEvent.evtype, deviceEvent.deviceid);
        return {};
    }
}

std::unique_ptr<Event::TableData> evget::EventTransformerLinux::buttonEvent(XIDeviceEvent& event) {
    int positionX = event.root_x;
    int positionY = event.root_y;


    if (mouseIds.contains(event.deviceid)) {
        //Event::MouseClick::MouseClickBuilder{}.
        //MouseEven
    }
    return std::unique_ptr<Event::TableData>();
}

void evget::EventTransformerLinux::refreshDeviceIds() {
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
        spdlog::warn("Devices with ids greater than 127 found. Set the type of these devices manually if their use is required.");
    }

    std::map<int, std::reference_wrapper<const XIDeviceInfo>> xi2Devices{};
    for (int i = 0; i < xi2NDevices; i++) {
        xi2Devices.emplace(xi2Info[i].deviceid, xi2Info[i]);
    }

    for (int i = 0; i < nDevices; i++) {
        const auto& device = info[i];
        int id = boost::numeric_cast<int>(device.id);

        if (!xi2Devices.contains(id)) {
            throw evget::UnsupportedOperationException{"Device id from XDeviceInfo not found in XIDeviceInfo."};
        }
        const auto& xi2Device = xi2Devices.at(id).get();

        if (xi2Device.enabled && device.type != None && (device.use == IsXExtensionPointer || device.use == IsXExtensionKeyboard || device.use == IsXExtensionDevice)) {
            auto type = std::unique_ptr<char[], decltype(&XFree)>(XGetAtomName(&display.get(), device.type), XFree);

            if (strcmp(type.get(), XI_MOUSE) == 0) {
                mouseIds.emplace(id, device.name);
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                keyboardIds.emplace(id, device.name);
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                touchscreenIds.emplace(id, device.name);
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                touchpadIds.emplace(id, device.name);
            } else {
                spdlog::info("Unsupported class type '{}' from XDeviceInfo for device '{}' with id {}.", type.get(), device.name, device.id);
            }

            setButtonMap(xi2Devices.at(id).get());
        }
    }
}

void evget::EventTransformerLinux::setButtonMap(const XIDeviceInfo& info) {
    const XIButtonClassInfo* buttonInfo = nullptr;
    for (int i = 0; i < info.num_classes; i++) {
        const auto* classInfo = info.classes[i];

        if (classInfo->type == ButtonClass) {
            buttonInfo = reinterpret_cast<const XIButtonClassInfo*>(classInfo);
            break;
        }
    }

    if (buttonInfo && buttonInfo->num_buttons > 0) {
        setButtonMap(*buttonInfo, info.deviceid);
    }
}

void evget::EventTransformerLinux::setButtonMap(const XIButtonClassInfo& buttonInfo, int id) {
    Display& displayCapture = display;
    auto deleter = [&displayCapture](XDevice* device) {
        XCloseDevice(&displayCapture, device);
    };
    auto device = std::unique_ptr<XDevice, decltype(deleter)>(XOpenDevice(&display.get(), id), deleter);

    if (device) {
        auto map = std::make_unique<unsigned char[]>(buttonInfo.num_buttons);
        XGetDeviceButtonMapping(&display.get(), device.get(), map.get(), buttonInfo.num_buttons);

        for (int i = 0; i < buttonInfo.num_buttons; i++) {
            if (!buttonInfo.labels[i]) {
                buttonMap[id][map[i]] = "None";
            } else {
                auto name = std::unique_ptr<char[], decltype(&XFree)>(XGetAtomName(&display.get(), buttonInfo.labels[i]), XFree);
                if (name) {
                    buttonMap[id][map[i]] = name.get();
                }
            }
        }
    }
}

evget::EventTransformerLinux::EventTransformerLinux(Display& display) : display{display} {
    refreshDeviceIds();
}
