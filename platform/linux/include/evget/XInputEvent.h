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

#include <memory>
#include <X11/Xlib.h>

namespace evget {
    class XInputEvent {
    public:
        class XEventCookieDeleter {
        public:
            explicit XEventCookieDeleter(Display& display);

            void operator()(XGenericEventCookie *pointer) const;

        private:
            std::reference_wrapper<Display> display;
        };

        using XEventPointer = std::unique_ptr<XGenericEventCookie, XEventCookieDeleter>;

        /**
         * Create the XInputEvent by getting the next event from the display. Events received depend on
         * the event mask set on the display.
         */
        explicit XInputEvent(Display& display);

        [[nodiscard]] int getEventType() const;

        /**
         * A non owning reference to the data in the event cookie.
         */
        template<typename T>
        const T& viewData() const;

    private:
        XEvent event{};
        XEventPointer cookie;
    };
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTEVENT_H
