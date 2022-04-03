`// MIT License
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
#include <functional>
#include <string>
#include <X11/extensions/XInput.h>
#include <memory>

namespace EvgetX11 {
    class XWrapper {
    public:
        class XDeviceDeleter {
        public:
            explicit XDeviceDeleter(Display& display);

            void operator()(XDevice *pointer) const;

        private:
            std::reference_wrapper<Display> display;
        };

        explicit XWrapper(Display& display);

        std::string lookupCharacter(XIDeviceEvent& event, KeySym& keySym);
        std::string keySymToString(KeySym keySym);
        std::unique_ptr<unsigned char[]> getDeviceButtonMapping(XDevice& device, int mapSize);

        std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> listInputDevices(int& ndevices);
        std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> queryDevice(int& ndevices);

        std::unique_ptr<char[], decltype(&XFree)> atomName(Atom atom);

        XEvent nextEvent();
        bool eventData(XGenericEventCookie& cookie);

    private:
        static std::unique_ptr<_XIC, decltype(&XDestroyIC)> createIC(Display& display, XIM xim);

        static constexpr int utf8MaxBytes = 4;

        std::reference_wrapper<Display> display;

        std::unique_ptr<_XIM, decltype(&XCloseIM)> xim = std::unique_ptr<_XIM, decltype(&XCloseIM)>{XOpenIM(&display.get(), nullptr, nullptr, nullptr), XCloseIM};
        std::unique_ptr<_XIC, decltype(&XDestroyIC)> xic = createIC(display, xim.get());
    };
}

#endif //EVGET_EVGETX11_INCLUDE_EVGETX11_XWRAPPER_H
`