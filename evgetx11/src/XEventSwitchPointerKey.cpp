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

#include "evgetx11/XEventSwitchPointerKey.h"

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <spdlog/spdlog.h>
#include <xorg/xserver-properties.h>

#include <cstring>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "evget/Event/Device.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XWrapper.h"
#include "evgetx11/XWrapperX11.h"

EvgetX11::XEventSwitchPointerKey::XEventSwitchPointerKey(XWrapper& xWrapper) : xWrapper{xWrapper} {}

void EvgetX11::XEventSwitchPointerKey::refreshDevices(
    int device_id,
    std::optional<int> pointer_id,
    evget::Event::Device device,
    const std::string& name,
    const XIDeviceInfo& info,
    EvgetX11::XEventSwitch& xEventSwitch
) {
    xEventSwitch.refreshDevices(device_id, pointer_id, device, name, info);

    if (pointer_id.has_value()) {
        this->pointer_id = *pointer_id;
    }

    std::vector<const XIScrollClassInfo*> scrollInfos{};
    std::vector<const XIValuatorClassInfo*> valuatorInfos{};
    auto classes = std::span(info.classes, info.num_classes);
    for (auto* classInfo : classes) {
        if (classInfo == nullptr) {
            spdlog::error("unexpected null pointer XIAnyClassInfo");
            continue;
        }

        // Reinterpret cast is required by X11.
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        if (classInfo->type == XIScrollClass) {
            scrollInfos.emplace_back(reinterpret_cast<const XIScrollClassInfo*>(classInfo));
        } else if (classInfo->type == XIValuatorClass) {
            valuatorInfos.emplace_back(reinterpret_cast<const XIValuatorClassInfo*>(classInfo));
        }
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    for (const auto* scrollInfo : scrollInfos) {
        scrollMap[device_id][scrollInfo->number] = *scrollInfo;
    }
    for (const auto* valuatorInfo : valuatorInfos) {
        auto valuatorName = xWrapper.get().atomName(valuatorInfo->label);
        if (valuatorName) {
            if (strcmp(valuatorName.get(), AXIS_LABEL_PROP_ABS_X) == 0 ||
                strcmp(valuatorName.get(), AXIS_LABEL_PROP_REL_X) == 0) {
                valuatorX[device_id] = valuatorInfo->number;
            } else if (strcmp(valuatorName.get(), AXIS_LABEL_PROP_ABS_Y) == 0 ||
                       strcmp(valuatorName.get(), AXIS_LABEL_PROP_REL_Y) == 0) {
                valuatorY[device_id] = valuatorInfo->number;
            }
        }
    }
}

std::map<int, int> EvgetX11::XEventSwitchPointerKey::getValuators(const XIValuatorState& valuatorState) {
    std::map<int, int> valuators{};
    auto* values = valuatorState.values;
    EvgetX11::XWrapperX11::onMasks(valuatorState.mask, valuatorState.mask_len, [&valuators, &values](int mask) {
        valuators.emplace(mask, (*values)++);
    });
    return valuators;
}
