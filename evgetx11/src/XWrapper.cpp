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

#include <X11/extensions/XInput2.h>
#include <spdlog/spdlog.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include "evgetx11/XWrapper.h"

EvgetX11::XWrapper::XWrapper(Display& display) : display{display} {
}

std::string EvgetX11::XWrapper::LookupCharacter(XIDeviceEvent& event, KeySym& keySym) {
    if (event.evtype == XI_KeyPress) {
        // Converts XIDeviceEvent to a XKeyEvent in order to leverage existing functions for determining KeySyms. Seems a
        // little bit hacky to do this conversion, however it should be okay as all the elements have a direct relationship.
        XKeyEvent keyEvent{
            .type = ButtonPress,
            .serial = event.serial,
            .send_event = event.send_event,
            .display = event.display,
            .window = event.event,
            .root = event.root,
            .subwindow = event.child,
            .time = event.time,
            .x = static_cast<int>(event.event_x),
            .y = static_cast<int>(event.event_y),
            .x_root = static_cast<int>(event.root_x),
            .y_root = static_cast<int>(event.root_y),
            .state = static_cast<unsigned int>(event.mods.effective),
            .keycode = static_cast<unsigned int>(event.detail),
            .same_screen = true
        };

        int bytes;
        std::array<char, utf8MaxBytes + 1> array{};
        if (xic) {
            Status status;
            bytes = Xutf8LookupString(xic.get(), &keyEvent, array.data(), utf8MaxBytes, &keySym, &status);
            if (status == XBufferOverflow) {
                spdlog::warn(
                    "Buffer overflowed when looking up string, falling back to encoding key events in ISO Latin-1."
                );
                bytes = XLookupString(&keyEvent, array.data(), utf8MaxBytes, &keySym, nullptr);
            }
        } else {
            bytes = XLookupString(&keyEvent, array.data(), utf8MaxBytes, &keySym, nullptr);
        }

        array[bytes] = '\0';
        return std::string{array.data()};
    }
    return {};
}

std::string EvgetX11::XWrapper::KeySymToString(KeySym keySym) {
    return XKeysymToString(keySym);
}

std::unique_ptr<_XIC, decltype(&XDestroyIC)> EvgetX11::XWrapper::createIC(Display& display, XIM xim) {
    if (xim) {
        auto xim_styles = std::unique_ptr<XIMStyles, decltype(&XFree)>{nullptr, XFree};
        auto values = XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL);
        if (values || !xim_styles) {
            spdlog::warn("The input method does not support any styles, falling back to encoding key events in ISO Latin-1.");
            return {nullptr, XDestroyIC};
        }

        for (int i = 0;  i < xim_styles->count_styles;  i++) {
            if (xim_styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) {
                Window window = XDefaultRootWindow(&display);
                return {XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, XNFocusWindow, window, nullptr), XDestroyIC};
            }
        }

        spdlog::warn("The input method does not support the PreeditNothing and StatusNothing style, falling back to encoding key events in ISO Latin-1.");
    }
    return {nullptr, XDestroyIC};
}
std::unique_ptr<unsigned char[]> EvgetX11::XWrapper::GetDeviceButtonMapping(
    XDevice& device,
    int mapSize
) {
    auto map = std::make_unique<unsigned char[]>(mapSize);
    XGetDeviceButtonMapping(&display.get(), &device, map.get(), mapSize);
    return map;
}

std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> EvgetX11::XWrapper::ListInputDevices(int& nDevices) {
    return {XListInputDevices(&display.get(), &nDevices), XFreeDeviceList};
}

std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> EvgetX11::XWrapper::QueryDevice(int& nDevices) {
    return {XIQueryDevice(&display.get(), XIAllDevices, &nDevices), XIFreeDeviceInfo};
}

EvgetX11::XWrapper::XDeviceDeleter::XDeviceDeleter(Display& display) : display{display} {
}

void EvgetX11::XWrapper::XDeviceDeleter::operator()(XDevice* pointer) const {
    XCloseDevice(&display.get(), pointer);
}