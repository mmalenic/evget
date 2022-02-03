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
#include "evget/EventTransformerLinux.h"

std::unique_ptr<Event::AbstractData> evget::EventTransformerLinux::transformEvent(evget::XInputEvent event) {
    return std::unique_ptr<Event::AbstractData>();
}

void evget::EventTransformerLinux::refreshDeviceIds() {
    int num_devices;
    auto info = std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>(XIQueryDevice(&display.get(), XIAllDevices, &num_devices),
        XIFreeDeviceInfo);

    for (int i = 0; i < num_devices; i++) {
        XIDeviceInfo device = info[i];

        if (device.enabled && (device.use == XISlavePointer || device.use == XISlaveKeyboard || device.use == XIFloatingSlave)) {
            bool deviceSet = false;
            bool hasButton = false;
            bool hasValuator = false;
            bool hasScroll = false;

            for (int j = 0; j < device.num_classes; j++) {
                XIAnyClassInfo* classInfo = device.classes[j];

                if (classInfo->type == XIKeyClass) {
                    keyboardIds.insert({device.deviceid, device.name});
                    deviceSet = true;
                    break;
                } else if (classInfo->type == XITouchClass) {
                    auto* touchInfo = reinterpret_cast<XITouchClassInfo*>(classInfo);

                    if (touchInfo->mode == XIDirectTouch) {
                        touchscreenIds.insert({device.deviceid, device.name});
                        deviceSet = true;
                        break;
                    } else if (touchInfo->mode == XIDependentTouch) {
                        touchpadIds.insert({device.deviceid, device.name});
                        deviceSet = true;
                        break;
                    }

                    spdlog::info("Unsupported touch class info mode '{}' for device '{}' with id {}.", touchInfo->mode, device.name, device.deviceid);
                } else if (classInfo->type == XIButtonClass) {
                    hasButton = true;
                } else if (classInfo->type == XIValuatorClass) {
                    hasValuator = true;
                } else if (classInfo->type == XIScrollClass) {
                    hasScroll = true;
                } else {
                    spdlog::info("Unsupported class type '{}' from XIDeviceInfo for device '{}' with id {}.", classInfo->type, device.name, device.deviceid);
                }
            }

            if (!deviceSet && hasButton && hasValuator && hasScroll) {
                mouseIds.insert({device.deviceid, device.name});
                break;
            }
            if (!deviceSet) {
                spdlog::info("Device '{}' with id {} not being used", device.name, device.deviceid);
            }
        }
    }
}

evget::EventTransformerLinux::EventTransformerLinux(Display& display) : display{display} {
    refreshDeviceIds();
}
