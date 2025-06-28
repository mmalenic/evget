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

#include <memory>

#include "XInputEvent.h"
#include "XWrapper.h"
#include "evgetcore/Error.h"

namespace EvgetX11 {

class XInputHandler {
public:
    static EvgetCore::Result<XInputHandler> build(XWrapper& xWrapper);

    /**
     * Get the next event.
     */
    XInputEvent getEvent() const;

private:
    explicit XInputHandler(XWrapper& xWrapper);

    static constexpr int versionMajor = 2;
    static constexpr int versionMinor = 2;

    std::reference_wrapper<XWrapper> xWrapper;

    static void setMask(XWrapper& xWrapper);

    static EvgetCore::Result<void> announceVersion(XWrapper& xWrapper);
};
}  // namespace EvgetX11

#endif  // EVGET_PLATFORM_LINUX_INCLUDE_EVGET_XINPUTHANDLER_H
