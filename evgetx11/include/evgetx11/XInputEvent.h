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
#include <chrono>
#include "evgetcore/Event/Data.h"
#include "XWrapperX11.h"
#include "evgetcore/Event/Field.h"
#include "XInputEventWrapper.h"

namespace EvgetX11 {
    template<typename T>
    class XInputEvent : public XInputEventWrapper<T> {
    public:
        [[nodiscard]] typename XInputEventWrapper<T>::Timestamp getTimestamp() const override;

        [[nodiscard]] const EvgetCore::Event::Field::DateTime &getDateTime() const override;

        const std::optional<std::reference_wrapper<T>> viewData() const override;

        /**
         * Create a XInputEvent by getting the next event from the display. Events received depend on
         * the event mask set on the display. This function will block if there are no events on the event
         * queue.
         */
        static XInputEvent nextEvent(XWrapper<XEventDeleter>& xWrapper);

    private:
        explicit XInputEvent(XWrapper<XEventDeleter>& xWrapper);

        XEvent event;
        typename XInputEventWrapper<T>::Timestamp timestamp;
        EvgetCore::Event::Field::DateTime dateTime;
        XWrapper<XEventDeleter>::XEventPointer cookie;
    };

    template<typename T>
    EvgetX11::XInputEvent<T>::XInputEvent(XWrapper<XEventDeleter>& xWrapper) : event{xWrapper.nextEvent()}, timestamp{std::chrono::steady_clock::now()}, dateTime{std::chrono::system_clock::now()}, cookie{xWrapper.eventData(event)} {
    }

    template<typename T>
    typename EvgetX11::XInputEventWrapper<T>::Timestamp EvgetX11::XInputEvent<T>::getTimestamp() const {
        return timestamp;
    }

    template<typename T>
    const EvgetCore::Event::Field::DateTime &EvgetX11::XInputEvent<T>::getDateTime() const {
        return dateTime;
    }

    template<typename T>
    const std::optional<std::reference_wrapper<T>> EvgetX11::XInputEvent<T>::viewData() const {
        return std::optional<std::reference_wrapper<T>>();
    }

    template<typename T>
    EvgetX11::XInputEvent<T> EvgetX11::XInputEvent<T>::nextEvent(XWrapper<XEventDeleter>& xWrapper) {
        return XInputEvent{xWrapper};
    }
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTEVENT_H
