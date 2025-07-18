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
#include <X11/extensions/XInput2.h>
#include <xorg/xserver-properties.h>

#include <cmath>
#include <map>
#include <ranges>
#include <unordered_map>

#include "XEventSwitch.h"
#include "XInputEvent.h"
#include "XWrapper.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/Event/MouseScroll.h"

namespace EvgetX11 {

class XEventSwitchPointerKey {
public:
    explicit XEventSwitchPointerKey(XWrapper& xWrapper);

    void refreshDevices(
        int device_id,
        std::optional<int> pointer_id,
        EvgetCore::Event::Device device,
        const std::string& name,
        const XIDeviceInfo& info,
        EvgetX11::XEventSwitch& xEventSwitch
    );
    bool switchOnEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetX11::XEventSwitch& xEventSwitch,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

private:
    static std::map<int, int> getValuators(const XIValuatorState& valuatorState);

    static void buttonEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Event::ButtonAction action,
        EvgetX11::XEventSwitch& xEventSwitch,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void keyEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetX11::XEventSwitch& xEventSwitch,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void motionEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetX11::XEventSwitch& xEventSwitch,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void scrollEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetX11::XEventSwitch& xEventSwitch,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

    std::reference_wrapper<XWrapper> xWrapper;

    std::unordered_map<int, std::unordered_map<int, XIScrollClassInfo>> scrollMap;
    std::unordered_map<int, std::optional<int>> valuatorX;
    std::unordered_map<int, std::optional<int>> valuatorY;

    int pointer_id{};
};

bool EvgetX11::XEventSwitchPointerKey::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetX11::XEventSwitch& xEventSwitch,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    switch (event.getEventType()) {
        case XI_RawMotion:
            motionEvent(event, data, xEventSwitch, getTime);
            scrollEvent(event, data, xEventSwitch, getTime);
            return true;
        case XI_RawButtonPress:
            buttonEvent(event, data, EvgetCore::Event::ButtonAction::Press, xEventSwitch, getTime);
            return true;
        case XI_RawButtonRelease:
            buttonEvent(event, data, EvgetCore::Event::ButtonAction::Release, xEventSwitch, getTime);
            return true;
        case XI_RawKeyPress:
        case XI_RawKeyRelease:
            keyEvent(event, data, xEventSwitch, getTime);
            return true;
        default:
            return false;
    }
}

void EvgetX11::XEventSwitchPointerKey::buttonEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Event::ButtonAction action,
    EvgetX11::XEventSwitch& xEventSwitch,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    auto button = xEventSwitch.getButtonName(raw_event.sourceid, raw_event.detail);
    // NOLINTBEGIN(hicpp-signed-bitwise)
    if (!xEventSwitch.hasDevice(raw_event.sourceid) || (raw_event.flags & XIPointerEmulated) ||
        button == BTN_LABEL_PROP_BTN_WHEEL_UP || button == BTN_LABEL_PROP_BTN_WHEEL_DOWN ||
        button == BTN_LABEL_PROP_BTN_HWHEEL_LEFT || button == BTN_LABEL_PROP_BTN_HWHEEL_RIGHT) {
        return;
    }
    // NOLINTEND(hicpp-signed-bitwise)

    xEventSwitch.addButtonEvent(raw_event, event.getTimestamp(), data, action, raw_event.detail, getTime);
}

void EvgetX11::XEventSwitchPointerKey::keyEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetX11::XEventSwitch& xEventSwitch,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    if (!xEventSwitch.hasDevice(raw_event.sourceid)) {
        return;
    }

    auto query_pointer = this->xWrapper.get().query_pointer(pointer_id);

    KeySym keySym = NoSymbol;

    const std::string character = xWrapper.get().lookupCharacter(raw_event, query_pointer, keySym);

    EvgetCore::Event::ButtonAction action = EvgetCore::Event::ButtonAction::Release;
    if (raw_event.evtype != XI_KeyRelease) {
        // NOLINTBEGIN(hicpp-signed-bitwise)
        action = (raw_event.flags & XIKeyRepeat) ? EvgetCore::Event::ButtonAction::Repeat
                                                 : EvgetCore::Event::ButtonAction::Press;
        // NOLINTEND(hicpp-signed-bitwise)
    }

    const std::string name = XWrapperX11::keySymToString(keySym);

    EvgetCore::Event::Key builder{};
    builder.interval(getTime(raw_event.time))
        .positionX(query_pointer.root_x)
        .positionY(query_pointer.root_y)
        .device(xEventSwitch.getDevice(raw_event.sourceid))
        .timestamp(event.getTimestamp())
        .action(action)
        .button(raw_event.detail)
        .character(character)
        .name(name);

    XEventSwitch::setModifierValue(query_pointer.modifier_state.effective, builder);
    xEventSwitch.setWindowFields(builder);

    xEventSwitch.setDeviceNameFields(builder, raw_event, query_pointer.screen_number);

    builder.build(data);
}

void EvgetX11::XEventSwitchPointerKey::scrollEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetX11::XEventSwitch& xEventSwitch,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    // NOLINTBEGIN(hicpp-signed-bitwise)
    if (!xEventSwitch.hasDevice(raw_event.sourceid) || !scrollMap.contains(raw_event.sourceid) ||
        raw_event.flags & XIPointerEmulated) {
        return;
    }
    // NOLINTEND(hicpp-signed-bitwise)

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
        if (info.scroll_type == XIScrollTypeHorizontal) {
            builder.horizontal(valuators[valuator]);
        } else {
            builder.vertical(valuators[valuator]);
        }
    }

    auto query_pointer = this->xWrapper.get().query_pointer(pointer_id);
    builder.interval(getTime(raw_event.time))
        .timestamp(event.getTimestamp())
        .device(xEventSwitch.getDevice(raw_event.sourceid))
        .positionX(query_pointer.root_x)
        .positionY(query_pointer.root_y);

    XEventSwitch::setModifierValue(query_pointer.modifier_state.effective, builder);
    xEventSwitch.setWindowFields(builder);

    xEventSwitch.setDeviceNameFields(builder, raw_event, query_pointer.screen_number);

    builder.build(data);
}

void EvgetX11::XEventSwitchPointerKey::motionEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetX11::XEventSwitch& xEventSwitch,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    // NOLINTBEGIN(hicpp-signed-bitwise)
    if (!xEventSwitch.hasDevice(raw_event.sourceid) || (raw_event.flags & XIPointerEmulated)) {
        return;
    }
    // NOLINTEND(hicpp-signed-bitwise)

    auto valuators = getValuators(raw_event.valuators);
    for (const auto& valuator : valuators | std::views::keys) {
        if (valuator == valuatorX[raw_event.sourceid] || valuator == valuatorY[raw_event.sourceid]) {
            xEventSwitch.addMotionEvent(raw_event, event.getTimestamp(), data, getTime);
            break;
        }
    }
}
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_COREXEVENTSWITCH_H
