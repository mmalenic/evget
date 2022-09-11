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
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
#include <xorg/xserver-properties.h>
#include <X11/extensions/XInput.h>
#include "evgetx11/XEventSwitchCore.h"
#include "evgetcore/Event/MouseMove.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/MouseClick.h"
#include "evgetcore/UnsupportedOperationException.h"

bool EvgetX11::XEventSwitchCore::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    EventData& data
) {
    switch (event.getEventType()) {
    case XI_Motion:
        motionEvent(event, timestamp, data);
        scrollEvent(event, timestamp, data);
        return true;
    case XI_ButtonPress:
        buttonEvent(event, timestamp, data, EvgetCore::Event::ButtonAction::Press);
        return true;
    case XI_ButtonRelease:
        buttonEvent(event, timestamp, data, EvgetCore::Event::ButtonAction::Release);
        return true;
    case XI_KeyPress:
    case XI_KeyRelease:
        keyEvent(event, timestamp, data);
        return true;
    default:
        return false;
    }
}

void EvgetX11::XEventSwitchCore::buttonEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<EvgetCore::Event::Data>& data, EvgetCore::Event::ButtonAction action) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    auto button = getButtonName(deviceEvent.deviceid, deviceEvent.detail);
    if (!containsDevice(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated) ||
            button == BTN_LABEL_PROP_BTN_WHEEL_UP ||
            button == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
            button == BTN_LABEL_PROP_BTN_HWHEEL_LEFT ||
            button == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }

    addButtonEvent(deviceEvent, timestamp, event.getDateTime(), data, action, deviceEvent.detail);
}

void EvgetX11::XEventSwitchCore::keyEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<EvgetCore::Event::Data>& data) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!containsDevice(deviceEvent.deviceid)) {
        return;
    }

    std::string character;
    KeySym keySym;

    character = xWrapper.get().lookupCharacter(deviceEvent, keySym);

    EvgetCore::Event::ButtonAction action = EvgetCore::Event::ButtonAction::Release;
    if (deviceEvent.evtype != XI_KeyRelease) {
        action = (deviceEvent.flags & XIKeyRepeat) ? EvgetCore::Event::ButtonAction::Repeat : EvgetCore::Event::ButtonAction::Press;
    }

    std::string name = XWrapper<XEventDeleter>::keySymToString(keySym);

    EvgetCore::Event::Key builder{};
    builder.time(timestamp).dateTime(event.getDateTime()).action(action).button(deviceEvent.detail).character(character).name(name);

    addTableData(data, builder.build(), createSystemData(deviceEvent, "KeySystemData"));
}

void EvgetX11::XEventSwitchCore::scrollEvent(
    const XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    std::vector<EvgetCore::Event::Data>& data
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!containsDevice(deviceEvent.deviceid) || !scrollMap.contains(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated)) {
        return;
    }

    EvgetCore::Event::MouseScroll builder{};
    auto valuators = getValuators(deviceEvent.valuators);
    for (const auto& [valuator, info] : scrollMap[deviceEvent.deviceid]) {
        if (!valuators.contains(valuator)) {
            continue;
        }

        auto value = valuators[valuator] - valuatorValues[deviceEvent.deviceid][valuator];
        valuatorValues[deviceEvent.deviceid][valuator] = value;

        if (info.type == XIScrollTypeVertical) {
            if (info.increment * value >= 0) {
                builder.down(value);
            } else {
                builder.up(value);
            }
        } else {
            if (info.increment * value >= 0) {
                builder.left(value);
            } else {
                builder.right(value);
            }
        }
    }

    builder.time(timestamp).dateTime(event.getDateTime()).device(getDevice(deviceEvent.deviceid))
    .positionX(deviceEvent.root_x).positionY(deviceEvent.root_y);

    data.emplace_back(builder.build());
    data.emplace_back(createSystemData(deviceEvent, "MouseScrollSystemData"));
}

void EvgetX11::XEventSwitchCore::motionEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<EvgetCore::Event::Data>& data) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!containsDevice(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated)) {
        return;
    }

    auto valuators = getValuators(deviceEvent.valuators);
    for (const auto& [valuator, value]: valuators) {
        if (valuator == valuatorX[deviceEvent.deviceid] || valuator == valuatorY[deviceEvent.deviceid]) {
            addMotionEvent(deviceEvent, timestamp, event.getDateTime(), data);
            break;
        }
    }
}


EvgetX11::XEventSwitchCore::XEventSwitchCore(XWrapper<XEventDeleter>& xWrapper) : XEventSwitchPointer(xWrapper), xWrapper{xWrapper} {
    setEvtypeName(XI_KeyPress, "KeyPress");
    setEvtypeName(XI_KeyRelease, "KeyRelease");
    setEvtypeName(XI_ButtonPress, "ButtonPress");
    setEvtypeName(XI_ButtonRelease, "ButtonRelease");
    setEvtypeName(XI_Motion, "Motion");
}

void EvgetX11::XEventSwitchCore::refreshDevices(
    int id,
    EvgetCore::Event::Device device,
    const std::string& name,
    const XIDeviceInfo& info
) {
    XEventSwitchPointer::refreshDevices(id, device, name, info);

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