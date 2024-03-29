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

#include "evgetx11/XEventSwitchTouch.h"

bool EvgetX11::XEventSwitchTouch::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
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
    EvgetCore::Event::Data& data,
    EvgetCore::Event::ButtonAction action,
    Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (xEventSwitchPointer.get().hasDevice(deviceEvent.deviceid)) {
        xEventSwitchPointer.get()
            .addButtonEvent(deviceEvent, getTime(deviceEvent.time), event.getTimestamp(), data, action, 1);
    }
}

void EvgetX11::XEventSwitchTouch::touchMotion(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto deviceEvent = event.viewData<XIDeviceEvent>();
    if (xEventSwitchPointer.get().hasDevice(deviceEvent.deviceid)) {
        xEventSwitchPointer.get().addMotionEvent(deviceEvent, getTime(deviceEvent.time), event.getTimestamp(), data);
    }
}

EvgetX11::XEventSwitchTouch::XEventSwitchTouch(XEventSwitch& xEventSwitchPointer
)
    : xEventSwitchPointer{xEventSwitchPointer} {
}