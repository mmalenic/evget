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

#ifndef EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTHANDLER_H
#define EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTHANDLER_H

#include <X11/X.h>
#include <X11/extensions/XInput2.h>
#include <memory>

namespace evget {

    class XInputHandler {
    public:
        class XDisplayDeleter {
        public:
            void operator()(Display *pointer) const {
                XCloseDisplay(pointer);
            }
        };

        class XEventCookieDeleter {
        public:
            explicit XEventCookieDeleter(std::shared_ptr<Display> display);

            void operator()(XGenericEventCookie *pointer) const {
                XFreeEventData(display.get(), pointer);
            }
        private:
            std::shared_ptr<Display> display;
        };

        class XInputEvent {
        public:
            friend class XInputHandler;

            using XEventPointer = std::unique_ptr<XGenericEventCookie, XEventCookieDeleter>;

            [[nodiscard]] int getEventType() const;

            /**
             * A non owning reference to the data in the event cookie.
             */
            template<typename T>
            const T& viewData() const;

        private:
            explicit XInputEvent(std::shared_ptr<Display> display);

            XEvent event{};
            XEventPointer cookie;
        };


        XInputHandler();

        /**
         * Get the next event.
         */
        XInputEvent getEvent();

    private:
        std::shared_ptr<Display> display{XOpenDisplay(nullptr), XDisplayDeleter{}};

        static void setMask(Display& display);
    };

    template<typename T>
    const T& evget::XInputHandler::XInputEvent::viewData() const {
        return *static_cast<T*>(cookie->data);
    }
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTHANDLER_H
