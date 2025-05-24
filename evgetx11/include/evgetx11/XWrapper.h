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

#ifndef EVGET_EVGETX11_INCLUDE_EVGETX11_XWRAPPER_H
#define EVGET_EVGETX11_INCLUDE_EVGETX11_XWRAPPER_H

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "DeleterWithDisplay.h"

namespace EvgetX11 {

struct XWindowDimensions {
    unsigned int width;
    unsigned int height;
};

using XEventPointer = std::unique_ptr<XGenericEventCookie, std::function<void(XGenericEventCookie*)>>;

struct QueryPointerResult {
    double root_x;
    double root_y;
    std::unique_ptr<unsigned char[], decltype(&XFree)> button_mask;
    XIModifierState modifier_state;
    XIGroupState group_state;
    int screen_number;
};

/**
 * An interface which wraps X11 library functions.
 */
class XWrapper {
public:
    virtual std::string lookupCharacter(const XIRawEvent& event, const QueryPointerResult& query_pointer, KeySym& keySym) = 0;
    virtual std::unique_ptr<unsigned char[]> getDeviceButtonMapping(int id, int mapSize) = 0;

    virtual std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> listInputDevices(int& ndevices) = 0;
    virtual std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> queryDevice(int& ndevices) = 0;

    virtual std::unique_ptr<char[], decltype(&XFree)> atomName(Atom atom) = 0;

    virtual std::optional<Window> getActiveWindow() = 0;
    virtual std::optional<Window> getFocusWindow() = 0;
    virtual std::optional<std::string> getWindowName(Window window) = 0;
    virtual std::optional<XWindowDimensions> getWindowSize(Window window) = 0;
    virtual std::optional<XWindowDimensions> getWindowPosition(Window window) = 0;

    virtual QueryPointerResult query_pointer(int device_id) = 0;

    virtual XEvent nextEvent() = 0;
    virtual XEventPointer eventData(XEvent& event) = 0;

    virtual Status queryVersion(int& major, int& minor) = 0;
    virtual void selectEvents(XIEventMask& mask) = 0;

    XWrapper() = default;
    virtual ~XWrapper() = default;

    XWrapper(XWrapper&&) noexcept = delete;
    XWrapper& operator=(XWrapper&&) noexcept = delete;

    XWrapper(const XWrapper&) = delete;
    XWrapper& operator=(const XWrapper&) = delete;

private:
    static constexpr int maskBits = 8;
};
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_XWRAPPER_H
