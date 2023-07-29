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

#ifndef EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTEVENT_H
#define EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTEVENT_H

#include <X11/Xlib.h>

#include <chrono>
#include <memory>

#include "XWrapper.h"
#include "evgetcore/Event/Schema.h"

namespace EvgetX11 {
class XInputEvent {
public:
    /**
     * Get the date time of the event.
     */
    [[nodiscard]] const EvgetCore::Event::Timestamp& getTimestamp() const;

    /**
     * Check if viewData and getEventType is safe to call.
     */
    [[nodiscard]] bool hasData() const;

    /**
     * Must check if data is available first with hasData.
     */
    [[nodiscard]] int getEventType() const;

    /**
     * A non owning reference to the data in the event cookie. Must check if data is available first
     * with hasData.
     */
    template <typename T>
    const T& viewData() const;

    /**
     * Create a XInputEvent by getting the next event from the display. Events received depend on
     * the event mask set on the display. This function will block if there are no events on the event
     * queue.
     */
    static XInputEvent nextEvent(XWrapper& xWrapper);

private:
    explicit XInputEvent(XWrapper& xWrapper);

    XEvent event;
    EvgetCore::Event::Timestamp timestamp;
    XEventPointer cookie;
};

template <typename T>
const T& EvgetX11::XInputEvent::viewData() const {
    return *static_cast<T*>(cookie->data);
}
}  // namespace EvgetX11

#endif  // EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTEVENT_H
