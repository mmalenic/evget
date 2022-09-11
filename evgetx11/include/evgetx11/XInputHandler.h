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

#include <X11/extensions/XInput2.h>
#include <memory>
#include "XInputEvent.h"
#include "XSetMask.h"

namespace EvgetX11 {

    class XInputHandler {
    public:
        explicit XInputHandler(XWrapper<XEventDeleter>& xWrapper, std::initializer_list<std::reference_wrapper<XSetMask>> maskSetters);

        /**
         * Get the next event.
         */
        XInputEvent getEvent();

    private:
        static constexpr int versionMajor = 2;
        static constexpr int versionMinor = 2;

        std::reference_wrapper<XWrapper<XEventDeleter>> xWrapper;

        static void setMask(XWrapper<XEventDeleter>& xWrapper, std::initializer_list<std::reference_wrapper<XSetMask>> maskSetters);

        static void announceVersion(XWrapper<XEventDeleter>& xWrapper);
    };
}

#endif //EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTHANDLER_H
