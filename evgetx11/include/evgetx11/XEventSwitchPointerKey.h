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
        XWrapper& xWrapper, XEventSwitch& xEventSwitch
    );

    void refreshDevices(int id, std::optional<int> pointer_id, EvgetCore::Event::Device device, const std::string& name, const XIDeviceInfo& info);
    bool switchOnEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

private:
    static std::map<int, int> getValuators(const XIValuatorState& valuatorState);

    void buttonEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Event::ButtonAction action,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void keyEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void motionEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void scrollEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

    std::reference_wrapper<XWrapper> xWrapper;
    std::reference_wrapper<XEventSwitch> xEventSwitch;

    std::unordered_map<int, std::unordered_map<int, XIScrollClassInfo>> scrollMap{};
    std::unordered_map<int, std::optional<int>> valuatorX{};
    std::unordered_map<int, std::optional<int>> valuatorY{};
    std::unordered_map<int, std::unordered_map<int, double>> valuatorValues{};

    int pointer_id{};
};

bool EvgetX11::XEventSwitchPointerKey::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    switch (event.getEventType()) {
        case XI_RawMotion:
            motionEvent(event, data, getTime);
            scrollEvent(event, data, getTime);
            return true;
        case XI_RawButtonPress:
            buttonEvent(event, data, EvgetCore::Event::ButtonAction::Press, getTime);
            return true;
        case XI_RawButtonRelease:
            buttonEvent(event, data, EvgetCore::Event::ButtonAction::Release, getTime);
            return true;
        case XI_RawKeyPress:
        case XI_RawKeyRelease:
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
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    auto button = xEventSwitch.get().getButtonName(raw_event.sourceid, raw_event.detail);
    if (!xEventSwitch.get().hasDevice(raw_event.sourceid) || (raw_event.flags & XIPointerEmulated) ||
        button == BTN_LABEL_PROP_BTN_WHEEL_UP || button == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        button == BTN_LABEL_PROP_BTN_HWHEEL_LEFT || button == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }

    xEventSwitch.get()
        .addButtonEvent(raw_event, event.getTimestamp(), data, action, raw_event.detail, getTime);
}

void EvgetX11::XEventSwitchPointerKey::keyEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    if (!xEventSwitch.get().hasDevice(raw_event.sourceid)) {
        return;
    }

    auto query_pointer = this->xWrapper.get().query_pointer(pointer_id);

    std::string character;
    KeySym keySym = NoSymbol;

    character = xWrapper.get().lookupCharacter(raw_event, query_pointer, keySym);

    EvgetCore::Event::ButtonAction action = EvgetCore::Event::ButtonAction::Release;
    if (raw_event.evtype != XI_KeyRelease) {
        action = (raw_event.flags & XIKeyRepeat) ? EvgetCore::Event::ButtonAction::Repeat
                                                   : EvgetCore::Event::ButtonAction::Press;
    }

    std::string name = XWrapperX11::keySymToString(keySym);

    EvgetCore::Event::Key builder{};
    builder.interval(getTime(raw_event.time))
        .positionX(query_pointer.root_x)
        .positionY(query_pointer.root_y)
        .device(xEventSwitch.get().getDevice(raw_event.sourceid))
        .timestamp(event.getTimestamp())
        .action(action)
        .button(raw_event.detail)
        .character(character)
        .name(name);

    XEventSwitch::setModifierValue(query_pointer.modifier_state.effective, builder);
    xEventSwitch.get().setWindowFields(builder);

    xEventSwitch.get().setDeviceNameFields(builder, raw_event);

    builder.build(data);
}

void EvgetX11::XEventSwitchPointerKey::scrollEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    if (!xEventSwitch.get().hasDevice(raw_event.sourceid) || !scrollMap.contains(raw_event.sourceid) ||
        raw_event.flags & XIPointerEmulated) {
        return;
    }

    EvgetCore::Event::MouseScroll builder{};
    auto valuators = getValuators(raw_event.valuators);
    std::unordered_map<int, XIScrollClassInfo> processedValuators{};
    for (const auto& [valuator, info] : scrollMap[raw_event.sourceid]) {
        if (valuators.contains(valuator)) {
            processedValuators.try_emplace(valuator, info);
        }
    }

    if (processedValuators.empty()) {
        return;
    }

    for (const auto& [valuator, info] : processedValuators) {
        auto value = valuatorValues[raw_event.sourceid][valuator] - valuators[valuator];
        valuatorValues[raw_event.sourceid][valuator] = valuators[valuator];

        if (info.scroll_type == XIScrollTypeHorizontal) {
            builder.horizontal(value);
        } else {
            builder.vertical(value);
        }
    }

    auto query_pointer = this->xWrapper.get().query_pointer(pointer_id);
    builder.interval(getTime(raw_event.time))
        .timestamp(event.getTimestamp())
        .device(xEventSwitch.get().getDevice(raw_event.sourceid))
        .positionX(query_pointer.root_x)
        .positionY(query_pointer.root_y);

    XEventSwitch::setModifierValue(query_pointer.modifier_state.effective, builder);
    xEventSwitch.get().setWindowFields(builder);

    xEventSwitch.get().setDeviceNameFields(builder, raw_event);

    builder.build(data);
}

void EvgetX11::XEventSwitchPointerKey::motionEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    if (!xEventSwitch.get().hasDevice(raw_event.sourceid) || (raw_event.flags & XIPointerEmulated)) {
        return;
    }

    auto valuators = getValuators(raw_event.valuators);
    for (const auto& [valuator, value] : valuators) {
        if (valuator == valuatorX[raw_event.sourceid] || valuator == valuatorY[raw_event.sourceid]) {
            xEventSwitch.get().addMotionEvent(raw_event, event.getTimestamp(), data, getTime);
            break;
        }
    }
}
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
