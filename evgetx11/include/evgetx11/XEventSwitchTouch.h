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

#ifndef EVGET_EVGETX11_INCLUDE_EVGETX11_TOUCHXEVENTSWITCH_H
#define EVGET_EVGETX11_INCLUDE_EVGETX11_TOUCHXEVENTSWITCH_H

#include "XDeviceRefresh.h"
#include "XEventSwitchCore.h"

namespace EvgetX11 {
class XEventSwitchTouch {
public:
    explicit XEventSwitchTouch(XEventSwitchPointer& xEventSwitchPointer, XDeviceRefresh& xDeviceRefresh);

    bool switchOnEvent(
        const XInputEvent& event,
        EventData& data,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

private:
    void touchButton(
        const XInputEvent& event,
        std::vector<EvgetCore::Event::Data>& data,
        EvgetCore::Event::ButtonAction action,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void touchMotion(
        const XInputEvent& event,
        std::vector<EvgetCore::Event::Data>& data,
        EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

    std::reference_wrapper<XEventSwitchPointer> xEventSwitchPointer;
    std::reference_wrapper<XDeviceRefresh> xDeviceRefresh;
};

bool EvgetX11::XEventSwitchTouch::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    EvgetX11::EventData& data,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    switch (event.getEventType()) {
        case XI_TouchBegin:
            touchMotion(event, data, getTime);
            touchButton(event, data, EvgetCore::Event::ButtonAction::Press, getTime);
            return true;
        case XI_TouchUpdate:
            touchMotion(event, data, getTime);
            return true;
        case XI_TouchEnd:
            touchMotion(event, data, getTime);
            touchButton(event, data, EvgetCore::Event::ButtonAction::Release, getTime);
            return true;
        default:
            return false;
    }
}

void EvgetX11::XEventSwitchTouch::touchButton(
    const EvgetX11::XInputEvent& event,
    std::vector<EvgetCore::Event::Data>& data,
    EvgetCore::Event::ButtonAction action,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (xDeviceRefresh.get().containsDevice(deviceEvent.deviceid)) {
        xEventSwitchPointer.get()
            .addButtonEvent(deviceEvent, getTime(deviceEvent.time), event.getTimestamp(), data, action, 1);
    }
}

void EvgetX11::XEventSwitchTouch::touchMotion(
    const EvgetX11::XInputEvent& event,
    std::vector<EvgetCore::Event::Data>& data,
    EvgetCore::Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (xDeviceRefresh.get().containsDevice(deviceEvent.deviceid)) {
        xEventSwitchPointer.get().addMotionEvent(deviceEvent, getTime(deviceEvent.time), event.getTimestamp(), data);
    }
}
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_TOUCHXEVENTSWITCH_H
