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

#ifndef EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
#define EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H

#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <spdlog/spdlog.h>
#include <xorg/xserver-properties.h>

#include <unordered_map>

#include "XDeviceRefresh.h"
#include "XEventSwitchPointer.h"
#include "XWrapper.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/MouseScroll.h"

namespace EvgetX11 {

template <XWrapper XWrapper>
class XEventSwitchCore {
public:
    explicit XEventSwitchCore(
        XWrapper& xWrapper,
        XEventSwitchPointer<XWrapper>& xEventSwitchPointer,
        XDeviceRefresh& xDeviceRefresh
    );

    void refreshDevices(int id, EvgetCore::Event::Device device, const std::string& name, const XIDeviceInfo& info);
    bool switchOnEvent(
        const XInputEvent& event,
        EventData& data,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

private:
    void buttonEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Event::ButtonAction action,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void keyEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void motionEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void scrollEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

    std::reference_wrapper<XWrapper> xWrapper;
    std::reference_wrapper<XEventSwitchPointer<XWrapper>> xEventSwitchPointer;
    std::reference_wrapper<XDeviceRefresh> xDeviceRefresh;

    std::unordered_map<int, std::unordered_map<int, XIScrollClassInfo>> scrollMap{};
    std::unordered_map<int, std::optional<int>> valuatorX{};
    std::unordered_map<int, std::optional<int>> valuatorY{};
    std::unordered_map<int, std::unordered_map<int, double>> valuatorValues{};
};

template <XWrapper XWrapper>
bool EvgetX11::XEventSwitchCore<XWrapper>::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    EventData& data,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    switch (event.getEventType()) {
        case XI_Motion:
            motionEvent(event, data, getTime);
            scrollEvent(event, data, getTime);
            return true;
        case XI_ButtonPress:
            buttonEvent(event, data, EvgetCore::Event::ButtonAction::Press, getTime);
            return true;
        case XI_ButtonRelease:
            buttonEvent(event, data, EvgetCore::Event::ButtonAction::Release, getTime);
            return true;
        case XI_KeyPress:
        case XI_KeyRelease:
            keyEvent(event, data, getTime);
            return true;
        default:
            return false;
    }
}

template <XWrapper XWrapper>
void EvgetX11::XEventSwitchCore<XWrapper>::buttonEvent(
    const XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Event::ButtonAction action,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    auto button = xEventSwitchPointer.get().getButtonName(deviceEvent.deviceid, deviceEvent.detail);
    if (!xDeviceRefresh.get().containsDevice(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated) ||
        button == BTN_LABEL_PROP_BTN_WHEEL_UP || button == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        button == BTN_LABEL_PROP_BTN_HWHEEL_LEFT || button == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }

    xEventSwitchPointer.get()
        .addButtonEvent(deviceEvent, event.getTimestamp(), data, action, deviceEvent.detail, getTime);
}

template <XWrapper XWrapper>
void EvgetX11::XEventSwitchCore<XWrapper>::keyEvent(
    const XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!xDeviceRefresh.get().containsDevice(deviceEvent.deviceid)) {
        return;
    }

    std::string character;
    KeySym keySym;

    character = xWrapper.get().lookupCharacter(deviceEvent, keySym);

    EvgetCore::Event::ButtonAction action = EvgetCore::Event::ButtonAction::Release;
    if (deviceEvent.evtype != XI_KeyRelease) {
        action = (deviceEvent.flags & XIKeyRepeat) ? EvgetCore::Event::ButtonAction::Repeat
                                                   : EvgetCore::Event::ButtonAction::Press;
    }

    std::string name = XWrapper::keySymToString(keySym);

    EvgetCore::Event::Key builder{};
    builder.interval(getTime(deviceEvent.time))
        .timestamp(event.getTimestamp())
        .action(action)
        .button(deviceEvent.detail)
        .character(character)
        .name(name);
    XEventSwitchPointer<XWrapper>::setModifierValue(deviceEvent.mods.effective, builder);
    xEventSwitchPointer.get().setWindowFields(builder);

    xEventSwitchPointer.get().setDeviceName(builder, deviceEvent);

    builder.build(data);
}

template <XWrapper XWrapper>
void EvgetX11::XEventSwitchCore<XWrapper>::scrollEvent(
    const XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!xDeviceRefresh.get().containsDevice(deviceEvent.deviceid) || !scrollMap.contains(deviceEvent.deviceid) ||
        (deviceEvent.flags & XIPointerEmulated)) {
        return;
    }

    EvgetCore::Event::MouseScroll builder{};
    auto valuators = XDeviceRefresh::getValuators(deviceEvent.valuators);
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

    builder.interval(getTime(deviceEvent.time))
        .timestamp(event.getTimestamp())
        .device(xDeviceRefresh.get().getDevice(deviceEvent.deviceid))
        .positionX(deviceEvent.root_x)
        .positionY(deviceEvent.root_y);
    XEventSwitchPointer<XWrapper>::setModifierValue(deviceEvent.mods.effective, builder);
    xEventSwitchPointer.get().setWindowFields(builder);

    xEventSwitchPointer.get().setDeviceName(builder, deviceEvent);

    builder.build(data);
}

template <XWrapper XWrapper>
void EvgetX11::XEventSwitchCore<XWrapper>::motionEvent(
    const XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!xDeviceRefresh.get().containsDevice(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated)) {
        return;
    }

    auto valuators = XDeviceRefresh::getValuators(deviceEvent.valuators);
    for (const auto& [valuator, value] : valuators) {
        if (valuator == valuatorX[deviceEvent.deviceid] || valuator == valuatorY[deviceEvent.deviceid]) {
            xEventSwitchPointer.get().addMotionEvent(deviceEvent, event.getTimestamp(), data, getTime);
            break;
        }
    }
}

template <XWrapper XWrapper>
EvgetX11::XEventSwitchCore<XWrapper>::XEventSwitchCore(
    XWrapper& xWrapper,
    XEventSwitchPointer<XWrapper>& xEventSwitchPointer,
    XDeviceRefresh& xDeviceRefresh
)
    : xWrapper{xWrapper}, xEventSwitchPointer{xEventSwitchPointer}, xDeviceRefresh{xDeviceRefresh} {
    xDeviceRefresh.setEvtypeName(XI_KeyPress, "KeyPress");
    xDeviceRefresh.setEvtypeName(XI_KeyRelease, "KeyRelease");
    xDeviceRefresh.setEvtypeName(XI_ButtonPress, "ButtonPress");
    xDeviceRefresh.setEvtypeName(XI_ButtonRelease, "ButtonRelease");
    xDeviceRefresh.setEvtypeName(XI_Motion, "Motion");
}

template <XWrapper XWrapper>
void EvgetX11::XEventSwitchCore<XWrapper>::refreshDevices(
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

}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
