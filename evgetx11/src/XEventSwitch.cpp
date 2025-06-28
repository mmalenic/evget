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

#include "evgetx11/XEventSwitch.h"

EvgetCore::Event::Device EvgetX11::XEventSwitch::getDevice(int device_id) const {
    return devices.at(device_id);
}

bool EvgetX11::XEventSwitch::hasDevice(int device_id) const {
    return devices.contains(device_id);
}

void EvgetX11::XEventSwitch::setButtonMap(const XIButtonClassInfo& buttonInfo, int device_id) {
    auto map = xWrapper.get().getDeviceButtonMapping(device_id, buttonInfo.num_buttons);
    if (map) {
        for (int i = 0; i < buttonInfo.num_buttons; i++) {
            if (buttonInfo.labels[i] != 0U) {
                auto name = xWrapper.get().atomName(buttonInfo.labels[i]);
                if (name) {
                    buttonMap[device_id][map[i]] = name.get();
                }
            }
        }
    }
}

void EvgetX11::XEventSwitch::refreshDevices(
    int device_id,
    std::optional<int> pointer_id,
    EvgetCore::Event::Device device,
    const std::string& name,
    const XIDeviceInfo& info
) {
    if (pointer_id.has_value()) {
        this->pointer_id = *pointer_id;
    }
    devices.emplace(device_id, device);
    idToName.emplace(device_id, name);

    const XIButtonClassInfo* buttonInfo = nullptr;
    for (int i = 0; i < info.num_classes; i++) {
        const auto* classInfo = info.classes[i];

        if (classInfo != nullptr && classInfo->type == XIButtonClass) {
            // Reinterpret cast is required by X11.
            // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
            buttonInfo = reinterpret_cast<const XIButtonClassInfo*>(classInfo);
            // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
            break;
        }
    }

    if (buttonInfo != nullptr && buttonInfo->num_buttons > 0) {
        setButtonMap(*buttonInfo, info.deviceid);
    }
}

EvgetX11::XEventSwitch::XEventSwitch(XWrapper& xWrapper) : xWrapper{xWrapper} {}

const std::string& EvgetX11::XEventSwitch::getButtonName(int device_id, int button) const {
    return buttonMap.at(device_id).at(button);
}
