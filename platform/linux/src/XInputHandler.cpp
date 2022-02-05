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

evget::XInputHandler::XInputHandler(Display& display) : display{display} {
    setMask(display);
}

void evget::XInputHandler::setMask(Display& display) {
    XIEventMask mask{};
    mask.deviceid = XIAllDevices;

    unsigned char eventMask[XI_LASTEVENT] = {0};
    XISetMask(eventMask, XI_ButtonPress);
    XISetMask(eventMask, XI_ButtonRelease);
    XISetMask(eventMask, XI_KeyPress);
    XISetMask(eventMask, XI_KeyRelease);
    XISetMask(eventMask, XI_Motion);
#if defined XI_TouchBegin && defined XI_TouchUpdate && defined XI_TouchEnd
    XISetMask(eventMask, XI_TouchBegin);
    XISetMask(eventMask, XI_TouchUpdate);
    XISetMask(eventMask, XI_TouchEnd);
#endif

    // Special events
    XISetMask(eventMask, XI_HierarchyChanged);
    XISetMask(eventMask, XI_DeviceChanged);

    mask.mask_len = sizeof(eventMask);
    mask.mask = eventMask;

    XISelectEvents(&display, XDefaultRootWindow(&display), &mask, 1);
    XSync(&display, false);
}

evget::XInputEvent evget::XInputHandler::getEvent() {
    return XInputEvent::nextEvent(display);
}