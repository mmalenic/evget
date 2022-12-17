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
#include "evgetcore/Util.h"

namespace EvgetX11 {
class XWrapper {
public:
    using XEventPointer = std::unique_ptr<XGenericEventCookie, std::function<void(XGenericEventCookie*)>>;

    virtual std::string lookupCharacter(const XIDeviceEvent& event, KeySym& keySym) = 0;
    virtual std::unique_ptr<unsigned char[]> getDeviceButtonMapping(int id, int mapSize) = 0;

    virtual std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> listInputDevices(int& ndevices) = 0;
    virtual std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> queryDevice(int& ndevices) = 0;

    virtual std::unique_ptr<char[], decltype(&XFree)> atomName(Atom atom) = 0;
    virtual std::optional<Atom> getAtom(const char* atomName) = 0;

    virtual std::unique_ptr<unsigned char*, decltype(&XFree)>
    getProperty(Atom atom, Window window, unsigned long& nItems, Atom& type, int& size) = 0;

    virtual std::optional<std::string> getWindowName(Window window) = 0;

    virtual XEvent nextEvent() = 0;
    virtual XEventPointer eventData(XEvent& event) = 0;

    virtual Status queryVersion(int& major, int& minor) = 0;
    virtual void selectEvents(XIEventMask& mask) = 0;

    static std::string keySymToString(KeySym keySym);
    static void setMask(unsigned char* mask, std::initializer_list<int> events);
    static void onMasks(const unsigned char* mask, int maskLen, EvgetCore::Util::Invocable<void, int> auto&& function);

    XWrapper() = default;
    virtual ~XWrapper() = default;

    XWrapper(XWrapper&&) noexcept = delete;
    XWrapper& operator=(XWrapper&&) noexcept = delete;

    XWrapper(const XWrapper&) = delete;
    XWrapper& operator=(const XWrapper&) = delete;

private:
    static constexpr int maskBits = 8;
};

void EvgetX11::XWrapper::onMasks(
    const unsigned char* mask,
    int maskLen,
    EvgetCore::Util::Invocable<void, int> auto&& function
) {
    for (int i = 0; i < maskLen * maskBits; i++) {
        if (XIMaskIsSet(mask, i)) {
            function(i);
        }
    }
}
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_XWRAPPER_H
