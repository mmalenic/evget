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

#include "evgetx11/XInputHandler.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

EvgetX11::XInputHandler::XInputHandler(
    XWrapper& xWrapper
)
    : xWrapper{xWrapper} {
}

EvgetCore::Storage::Result<void> EvgetX11::XInputHandler::announceVersion(XWrapper& xWrapper) {
    int major = versionMajor;
    int minor = versionMinor;

    Status status = xWrapper.queryVersion(major, minor);
    if (status == Success) {
        spdlog::info("XI2 is supported with version {}.{}", major, minor);
        return {};
    }

    return EvgetCore::Storage::Err{
        {.errorType = EvgetCore::Storage::ErrorType::EventHandlerError,
         .message = fmt::format("XI2 is not supported, only version {}.{} is available.", major, minor)}
    };
}

void EvgetX11::XInputHandler::setMask(XWrapper& xWrapper, std::vector<std::reference_wrapper<XSetMask>> maskSetters) {
    XIEventMask mask{};
    mask.deviceid = XIAllDevices;

    unsigned char eventMask[XI_LASTEVENT] = {0};
    for (const auto& maskSetter : maskSetters) {
        maskSetter.get().setMask(eventMask);
    }

    mask.mask_len = sizeof(eventMask);
    mask.mask = eventMask;

    xWrapper.selectEvents(mask);
}

EvgetCore::Storage::Result<EvgetX11::XInputHandler>
EvgetX11::XInputHandler::build(XWrapper& xWrapper, std::vector<std::reference_wrapper<XSetMask>> maskSetters) {
    return announceVersion(xWrapper).transform([&xWrapper, &maskSetters] {
        setMask(xWrapper, maskSetters);
        return XInputHandler{xWrapper};
    });
}

EvgetX11::XInputEvent EvgetX11::XInputHandler::getEvent() {
    return XInputEvent::nextEvent(xWrapper.get());
}