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

#include "XEventSwitch.h"
#include "XInputEvent.h"
#include "XWrapper.h"
#include <evgetcore/Error.h>

namespace EvgetX11 {

class XEventSwitchTouch {
public:
    explicit XEventSwitchTouch(XEventSwitch& xEventSwitchPointer);

    bool switchOnEvent(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

private:
    void touchButton(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Event::ButtonAction action,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void touchMotion(
        const XInputEvent& event,
        EvgetCore::Event::Data& data,
        EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

    std::reference_wrapper<XEventSwitch> xEventSwitchPointer;
};

bool EvgetX11::XEventSwitchTouch::switchOnEvent(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    switch (event.getEventType()) {
        case XI_RawTouchBegin:
            touchMotion(event, data, getTime);
            touchButton(event, data, EvgetCore::Event::ButtonAction::Press, getTime);
        return true;
        case XI_RawTouchUpdate:
            touchMotion(event, data, getTime);
        return true;
        case XI_RawTouchEnd:
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
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    if (xEventSwitchPointer.get().hasDevice(raw_event.sourceid)) {
        xEventSwitchPointer.get()
            .addButtonEvent(raw_event, event.getTimestamp(), data, action, raw_event.detail, getTime);
    }
}

void EvgetX11::XEventSwitchTouch::touchMotion(
    const EvgetX11::XInputEvent& event,
    EvgetCore::Event::Data& data,
    EvgetCore::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    auto raw_event = event.viewData<XIRawEvent>();
    if (xEventSwitchPointer.get().hasDevice(raw_event.sourceid)) {
        xEventSwitchPointer.get().addMotionEvent(raw_event, event.getTimestamp(), data, getTime);
    }
}
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_TOUCHXEVENTSWITCH_H
