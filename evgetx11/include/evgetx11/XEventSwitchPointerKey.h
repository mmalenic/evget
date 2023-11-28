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
#include <cmath>

#include "XEventSwitch.h"
#include "XWrapper.h"
#include "XInputEvent.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/MouseScroll.h"

namespace EvgetX11 {

class XEventSwitchPointerKey {
public:
    explicit XEventSwitchPointerKey(
        XWrapper& xWrapper, XEventSwitch& xEventSwitchPointer
    );

    void refreshDevices(int id, EvgetCore::Event::Device device, const std::string& name, const XIDeviceInfo& info);
    bool switchOnEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

private:
    static std::map<int, int> getValuators(const XIValuatorState& valuatorState);

    void buttonEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Event::ButtonAction action,
        Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void keyEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void motionEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void scrollEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

    std::reference_wrapper<XWrapper> xWrapper;
    std::reference_wrapper<XEventSwitch> xEventSwitchPointer;

    std::unordered_map<int, std::unordered_map<int, XIScrollClassInfo>> scrollMap{};
    std::unordered_map<int, std::optional<int>> valuatorX{};
    std::unordered_map<int, std::optional<int>> valuatorY{};
    std::unordered_map<int, std::unordered_map<int, double>> valuatorValues{};
};

bool EvgetX11::XEventSwitchPointerKey::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
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

void EvgetX11::XEventSwitchPointerKey::buttonEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Event::ButtonAction action,
    Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    auto button = xEventSwitchPointer.get().getButtonName(deviceEvent.deviceid, deviceEvent.detail);
    if (!xEventSwitchPointer.get().hasDevice(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated) ||
        button == BTN_LABEL_PROP_BTN_WHEEL_UP || button == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        button == BTN_LABEL_PROP_BTN_HWHEEL_LEFT || button == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }

    xEventSwitchPointer.get()
        .addButtonEvent(deviceEvent, event.getTimestamp(), data, action, deviceEvent.detail, getTime);
}

void EvgetX11::XEventSwitchPointerKey::keyEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!xEventSwitchPointer.get().hasDevice(deviceEvent.deviceid)) {
        return;
    }

    std::string character;
    KeySym keySym = NoSymbol;

    character = xWrapper.get().lookupCharacter(deviceEvent, keySym);

    EvgetCore::Event::ButtonAction action = EvgetCore::Event::ButtonAction::Release;
    if (deviceEvent.evtype != XI_KeyRelease) {
        action = (deviceEvent.flags & XIKeyRepeat) ? EvgetCore::Event::ButtonAction::Repeat
                                                   : EvgetCore::Event::ButtonAction::Press;
    }

    std::string name = XWrapperX11::keySymToString(keySym);

    EvgetCore::Event::Key builder{};
    builder.interval(getTime(deviceEvent.time))
        .positionX(deviceEvent.root_x)
        .positionY(deviceEvent.root_y)
        .device(xEventSwitchPointer.get().getDevice(deviceEvent.deviceid))
        .timestamp(event.getTimestamp())
        .action(action)
        .button(deviceEvent.detail)
        .character(character)
        .name(name);

    XEventSwitch::setModifierValue(deviceEvent.mods.effective, builder);
    xEventSwitchPointer.get().setWindowFields(builder);

    xEventSwitchPointer.get().setDeviceNameFields(builder, deviceEvent);

    builder.build(data);
}

void EvgetX11::XEventSwitchPointerKey::scrollEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!xEventSwitchPointer.get().hasDevice(deviceEvent.deviceid) || !scrollMap.contains(deviceEvent.deviceid) ||
        deviceEvent.flags & XIPointerEmulated) {
        return;
    }

    EvgetCore::Event::MouseScroll builder{};
    auto valuators = getValuators(deviceEvent.valuators);
    std::unordered_map<int, XIScrollClassInfo> processedValuators{};
    for (const auto& [valuator, info] : scrollMap[deviceEvent.deviceid]) {
        if (valuators.contains(valuator)) {
            processedValuators.try_emplace(valuator, info);
        }
    }

    if (processedValuators.empty()) {
        return;
    }

    for (const auto& [valuator, info] : processedValuators) {
        auto value = valuatorValues[deviceEvent.deviceid][valuator] - valuators[valuator];
        valuatorValues[deviceEvent.deviceid][valuator] = valuators[valuator];

        if (info.scroll_type == XIScrollTypeHorizontal) {
            builder.horizontal(value);
        } else {
            builder.vertical(value);
        }
    }

    builder.interval(getTime(deviceEvent.time))
        .timestamp(event.getTimestamp())
        .device(xEventSwitchPointer.get().getDevice(deviceEvent.deviceid))
        .positionX(deviceEvent.root_x)
        .positionY(deviceEvent.root_y);

    XEventSwitch::setModifierValue(deviceEvent.mods.effective, builder);
    xEventSwitchPointer.get().setWindowFields(builder);

    xEventSwitchPointer.get().setDeviceNameFields(builder, deviceEvent);

    builder.build(data);
}

void EvgetX11::XEventSwitchPointerKey::motionEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    Async::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (!xEventSwitchPointer.get().hasDevice(deviceEvent.deviceid) || (deviceEvent.flags & XIPointerEmulated)) {
        return;
    }

    auto valuators = getValuators(deviceEvent.valuators);
    for (const auto& [valuator, value] : valuators) {
        if (valuator == valuatorX[deviceEvent.deviceid] || valuator == valuatorY[deviceEvent.deviceid]) {
            xEventSwitchPointer.get().addMotionEvent(deviceEvent, event.getTimestamp(), data, getTime);
            break;
        }
    }
}
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
