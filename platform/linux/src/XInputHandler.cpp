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
#include "evget/XInputHandler.h"

evget::XInputHandler::XInputHandler() {
    setMask(display, window);
}

void evget::XInputHandler::setMask(Display* display, Window& window) {
    XIEventMask mask{};
    mask.deviceid = XIAllDevices;

    unsigned char eventMask[XI_LASTEVENT] = {0};
    XISetMask(eventMask, XI_ButtonPress);
    XISetMask(eventMask, XI_ButtonRelease);
    XISetMask(eventMask, XI_KeyPress);
    XISetMask(eventMask, XI_KeyRelease);
    XISetMask(eventMask, XI_Motion);
#ifdef XI_TouchBegin
    XISetMask(eventMask, XI_TouchBegin);
#endif
#ifdef XI_TouchUpdate
    XISetMask(eventMask, XI_TouchUpdate);
#endif
#ifdef XI_TouchEnd
    XISetMask(eventMask, XI_TouchEnd);
#endif
#ifdef XI_TouchOwnership
    XISetMask(eventMask, XI_TouchOwnership);
#endif

    mask.mask_len = sizeof(eventMask);
    mask.mask = eventMask;

    XISelectEvents(display, window, &mask, 1);
    XSync(display, false);
}

evget::XInputHandler::XEventPointer evget::XInputHandler::getEvent() {
    XEvent event;

    XNextEvent(display, &event);
    if (XGetEventData(display, &event.xcookie) && (&event.xcookie)->type == GenericEvent) {
        spdlog::trace(fmt::format("Event type {} captured.", (&event.xcookie)->type));
        return {&event.xcookie, XEventCookieDeleter(display)};
    }
    return {nullptr, XEventCookieDeleter(display)};
}

evget::XInputHandler::XEventCookieDeleter::XEventCookieDeleter(Display* display) : display{display} {
}
