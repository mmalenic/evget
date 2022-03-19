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

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include "evgetx11/XInputEvent.h"

EvgetX11::XInputEvent::XInputEvent(Display& display) : cookie{nullptr, XEventCookieDeleter{display}} {
    XNextEvent(&display, &event);
    timestamp = std::chrono::steady_clock::now();
    if (XGetEventData(&display, &event.xcookie) && (&event.xcookie)->type == GenericEvent) {
        spdlog::trace(fmt::format("Event type {} captured.", (&event.xcookie)->type));
        cookie.reset(&event.xcookie);
    }
}

bool EvgetX11::XInputEvent::hasData() const {
    return cookie != nullptr;
}

int EvgetX11::XInputEvent::getEventType() const {
    return cookie->evtype;
}

EvgetX11::XInputEvent::XEventCookieDeleter::XEventCookieDeleter(Display& display) : display{display} {
}

void EvgetX11::XInputEvent::XEventCookieDeleter::operator()(XGenericEventCookie* pointer) const {
    XFreeEventData(&display.get(), pointer);
}

EvgetX11::XInputEvent EvgetX11::XInputEvent::nextEvent(Display& display) {
    return XInputEvent{display};
}

EvgetX11::XInputEvent::Timestamp EvgetX11::XInputEvent::getTimestamp() const {
    return timestamp;
}