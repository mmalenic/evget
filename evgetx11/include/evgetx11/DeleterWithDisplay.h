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

#ifndef EVGET_EVGETX11_INCLUDE_EVGETX11_XEVENTCOOKIEDELETER_H
#define EVGET_EVGETX11_INCLUDE_EVGETX11_XEVENTCOOKIEDELETER_H

#include <X11/Xlib.h>
#include <functional>

namespace EvgetX11 {
    template <auto F>
    class DeleterWithDisplay {
    public:
        explicit DeleterWithDisplay(Display& display);

        template <typename T>
        void operator()(T *pointer) const;

    private:
        std::reference_wrapper<Display> display;
    };

    template<auto F>
    template<typename T>
    void EvgetX11::DeleterWithDisplay<F>::operator()(T *pointer) const {
        F(&display.get(), pointer);
    }

    template<auto fn>
    DeleterWithDisplay<fn>::DeleterWithDisplay(Display &display) : display{display} {}
}

#endif //EVGET_EVGETX11_INCLUDE_EVGETX11_XEVENTCOOKIEDELETER_H
