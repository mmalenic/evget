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

#include "evgetx11/XEventSwitchCore.h"

#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <spdlog/spdlog.h>
#include <xorg/xserver-properties.h>

#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/MouseClick.h"
#include "evgetcore/Event/MouseMove.h"
#include "evgetcore/UnsupportedOperationException.h"

EvgetX11::XEventSwitchCore::XEventSwitchCore(
    XWrapper& xWrapper,
    XEventSwitchPointer& xEventSwitchPointer,
    XDeviceRefresh& xDeviceRefresh
)
    : xWrapper{xWrapper}, xEventSwitchPointer{xEventSwitchPointer}, xDeviceRefresh{xDeviceRefresh} {
    xDeviceRefresh.setEvtypeName(XI_KeyPress, "KeyPress");
    xDeviceRefresh.setEvtypeName(XI_KeyRelease, "KeyRelease");
    xDeviceRefresh.setEvtypeName(XI_ButtonPress, "ButtonPress");
    xDeviceRefresh.setEvtypeName(XI_ButtonRelease, "ButtonRelease");
    xDeviceRefresh.setEvtypeName(XI_Motion, "Motion");
}

void EvgetX11::XEventSwitchCore::refreshDevices(
    int id,
    EvgetCore::Event::Device device,
    const std::string& name,
    const XIDeviceInfo& info
) {
    xEventSwitchPointer.get().refreshDevices(id, device, name, info);

    std::vector<const XIScrollClassInfo*> scrollInfos{};
    std::vector<const XIValuatorClassInfo*> valuatorInfos{};
    for (int i = 0; i < info.num_classes; i++) {
        const auto* classInfo = info.classes[i];

        if (classInfo->type == XIScrollClass) {
            scrollInfos.emplace_back(reinterpret_cast<const XIScrollClassInfo*>(classInfo));
        } else if (classInfo->type == XIValuatorClass) {
            valuatorInfos.emplace_back(reinterpret_cast<const XIValuatorClassInfo*>(classInfo));
        }
    }

    for (auto scrollInfo : scrollInfos) {
        scrollMap[id][scrollInfo->number] = *scrollInfo;
    }
    for (auto valuatorInfo : valuatorInfos) {
        auto valuatorName = xWrapper.get().atomName(valuatorInfo->label);
        if (valuatorName) {
            if (strcmp(valuatorName.get(), AXIS_LABEL_PROP_ABS_X) == 0) {
                valuatorX[id] = valuatorInfo->number;
            } else if (strcmp(valuatorName.get(), AXIS_LABEL_PROP_ABS_Y) == 0) {
                valuatorY[id] = valuatorInfo->number;
            }
        }
        valuatorValues[id][valuatorInfo->number] = valuatorInfo->value;
    }
}