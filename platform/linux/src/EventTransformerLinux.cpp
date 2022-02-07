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
#include "evget/EventTransformerLinux.h"

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
    if (mouseIds.contains(event.deviceid)) {

    }
    return std::unique_ptr<Event::TableData>();
}

void evget::EventTransformerLinux::refreshDeviceIds() {
    int num_devices;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>(XListInputDevices(&display.get(), &num_devices),
        XFreeDeviceList);
    for (int i = 0; i < num_devices; i++) {
        XDeviceInfo& device = info[i];

        if (device.type != None && (device.use == IsXExtensionPointer || device.use == IsXExtensionKeyboard || device.use == IsXExtensionDevice)) {
            auto type = std::unique_ptr<char[], decltype(&XFree)>(XGetAtomName(&display.get(), device.type), XFree);

            if (strcmp(type.get(), XI_MOUSE) == 0) {
                mouseIds.emplace(device.id, device.name);
            } else if (strcmp(type.get(), XI_KEYBOARD) == 0) {
                keyboardIds.emplace(device.id, device.name);
            } else if (strcmp(type.get(), XI_TOUCHSCREEN) == 0) {
                touchscreenIds.emplace(device.id, device.name);
            } else if (strcmp(type.get(), XI_TOUCHPAD) == 0) {
                touchpadIds.emplace(device.id, device.name);
            } else {
                spdlog::info("Unsupported class type '{}' from XDeviceInfo for device '{}' with id {}.", type.get(), device.name, device.id);
            }
        }
    }
}

void evget::EventTransformerLinux::setButtonMap() {

}

evget::EventTransformerLinux::EventTransformerLinux(Display& display) : display{display} {
    refreshDeviceIds();
}
