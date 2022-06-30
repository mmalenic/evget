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

bool EvgetX11::XEventSwitchCore::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    std::chrono::nanoseconds timestamp,
    EventData& data
) {
    auto type = event.getEventType();
    if (motionEvent(event, timestamp, type, data)) {
        return true;
    }

    switch (type) {
    case XI_RawMotion:
        rawScrollEvent = scrollEvent(event, timestamp);
        return true;
    case XI_ButtonPress:
        buttonEvent(event, timestamp, data, EvgetCore::Event::Button::ButtonAction::Press);
        return true;
    case XI_ButtonRelease:
        buttonEvent(event, timestamp, data, EvgetCore::Event::Button::ButtonAction::Release);
        return true;
    case XI_KeyPress:
    case XI_KeyRelease:
        keyEvent(event, timestamp, data);
        return true;
    default:
        return false;
    }
}

void EvgetX11::XEventSwitchCore::buttonEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data, EvgetCore::Event::Button::ButtonAction action) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    auto button = getButtonName(deviceEvent.deviceid, deviceEvent.detail);
    if (!devicesContains(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated) ||
            button == BTN_LABEL_PROP_BTN_WHEEL_UP ||
            button == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
            button == BTN_LABEL_PROP_BTN_HWHEEL_LEFT ||
            button == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }

    addButtonEvent(deviceEvent, timestamp, data, action, deviceEvent.detail);
}

void EvgetX11::XEventSwitchCore::keyEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!devicesContains(deviceEvent.deviceid)) {
        return;
    }

    std::string character;
    KeySym keySym;

    character = xWrapper.get().lookupCharacter(deviceEvent, keySym);

    EvgetCore::Event::Button::ButtonAction action = EvgetCore::Event::Button::ButtonAction::Release;
    if (deviceEvent.evtype != XI_KeyRelease) {
        action = (deviceEvent.flags & XIKeyRepeat) ? EvgetCore::Event::Button::ButtonAction::Repeat : EvgetCore::Event::Button::ButtonAction::Press;
    }

    std::string name = XWrapper::keySymToString(keySym);

    EvgetCore::Event::Key::KeyBuilder builder{};
    builder.time(timestamp).action(action).button(deviceEvent.detail).character(character).name(name);

    addTableData(data, builder.build(), createSystemDataWithRoot(deviceEvent, "KeySystemData"));
}

std::unique_ptr<EvgetCore::Event::MouseScroll> EvgetX11::XEventSwitchCore::scrollEvent(
    const XInputEvent& event,
    std::chrono::nanoseconds timestamp
) {
    auto rawEvent = event.viewData<XIRawEvent>();
    if (!devicesContains(rawEvent.sourceid) || !scrollMap.contains(rawEvent.sourceid) || (rawEvent.flags & XIPointerEmulated)) {
        return {};
    }

    EvgetCore::Event::MouseScroll::MouseScrollBuilder builder{};
    auto valuators = getValuators(rawEvent.valuators);
    bool isScrollEvent = false;
    for (const auto& [valuator, info] : scrollMap[rawEvent.sourceid]) {
        if (!valuators.contains(valuator)) {
            continue;
        }
        if (info.type == XIScrollTypeVertical) {
            if (info.increment * valuators[valuator] >= 0) {
                builder.down(valuators[valuator]);
            } else {
                builder.up(valuators[valuator]);
            }
        } else {
            if (info.increment * valuators[valuator] >= 0) {
                builder.left(valuators[valuator]);
            } else {
                builder.right(valuators[valuator]);
            }
        }
        isScrollEvent = true;
    }

    return isScrollEvent ? builder.time(timestamp).device(getDevice(rawEvent.sourceid)).build() : nullptr;
}

bool EvgetX11::XEventSwitchCore::motionEvent(const XInputEvent& event, std::chrono::nanoseconds timestamp, int type, std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data) {
    bool isMotion = type == XI_Motion;
    if (isMotion) {
        auto deviceEvent = event.viewData<XIDeviceEvent>();
        if (devicesContains(deviceEvent.deviceid) && !(deviceEvent.flags & XIPointerEmulated)) {
            motionEvent(timestamp, data, deviceEvent);
        }
    }

    if (rawScrollEvent) {
        spdlog::warn("Missing complimentary scroll event after XIRawEvent.");
        data.emplace_back(EvgetCore::Event::TableData::TableDataBuilder{}.genericData(std::move(rawScrollEvent)).build());
    }

    return isMotion;
}

void EvgetX11::XEventSwitchCore::motionEvent(
    std::chrono::nanoseconds timestamp,
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data,
    const XIDeviceEvent& deviceEvent
) {
    auto valuators = getValuators(deviceEvent.valuators);
    const auto& scrollValuators =
        scrollMap.contains(deviceEvent.deviceid) ? scrollMap[deviceEvent.deviceid] : std::map<int, XIScrollClassInfo>{};

    bool hasScroll = false;
    bool hasMotion = false;
    for (const auto& [valuator, value]: valuators) {
        if (!hasMotion && (valuator == valuatorX || valuator == valuatorY)) {
            addMotionEvent(deviceEvent, timestamp, data);
            hasMotion = true;
        } else if (!hasScroll) {
            hasScroll = scrollEvent(deviceEvent, data, scrollValuators, valuator);
        }
    }
}

bool EvgetX11::XEventSwitchCore::scrollEvent(
    const XIDeviceEvent& event,
    std::vector<std::unique_ptr<EvgetCore::Event::TableData>>& data,
    const std::map<int, XIScrollClassInfo>& scrollValuators,
    const int valuator
) {
    for (const auto& [scrollValuator, _]: scrollValuators) {
        if (valuator == scrollValuator) {
            addTableData(data, std::move(rawScrollEvent), createSystemDataWithRoot(event, "MouseScrollSystemData"));
            return true;
        }
    }
    return false;
}

EvgetX11::XEventSwitchCore::XEventSwitchCore(XWrapper& xWrapper) : XEventSwitchPointer(xWrapper), xWrapper{xWrapper} {
    setEvtypeName(XI_KeyPress, "KeyPress");
    setEvtypeName(XI_KeyRelease, "KeyRelease");
    setEvtypeName(XI_ButtonPress, "ButtonPress");
    setEvtypeName(XI_ButtonRelease, "ButtonRelease");
    setEvtypeName(XI_Motion, "Motion");
}

void EvgetX11::XEventSwitchCore::refreshDevices(
    int id,
    EvgetCore::Event::Common::Device device,
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
        scrollMap[info.deviceid][scrollInfo->number] = *scrollInfo;
    }
    for (auto valuatorInfo : valuatorInfos) {
        auto name = xWrapper.get().atomName(valuatorInfo->label);
        if (name) {
            if (strcmp(name.get(), AXIS_LABEL_PROP_ABS_X) == 0) {
                valuatorX = valuatorInfo->number;
            } else if (strcmp(name.get(), AXIS_LABEL_PROP_ABS_Y) == 0) {
                valuatorY = valuatorInfo->number;
            }
            valuatorNames.emplace(valuatorInfo->number, name.get());
        }
    }
}
