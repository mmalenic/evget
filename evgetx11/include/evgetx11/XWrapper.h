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

struct XWindowDimensions {
    unsigned int width;
    unsigned int height;
};

using XEventPointer = std::unique_ptr<XGenericEventCookie, std::function<void(XGenericEventCookie*)>>;

/**
 * Concept which defines the interface for a wrapper around X11 functions.
 */
template <typename T>
concept XWrapper =
    requires(T wrapper, const XIDeviceEvent& event, KeySym& keySym) {
        { wrapper.lookupCharacter(event, keySym) } -> std::convertible_to<std::string>;
    } && requires(T wrapper, int id, int mapSize) {
             { wrapper.getDeviceButtonMapping(id, mapSize) } -> std::convertible_to<std::unique_ptr<unsigned char[]>>;
         } && requires(T wrapper, int& ndevices) {
                  {
                      wrapper.listInputDevices(ndevices)
                  } -> std::convertible_to<std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>>;
              } && requires(T wrapper, int& ndevices) {
                       {
                           wrapper.queryDevice(ndevices)
                       } -> std::convertible_to<std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>>;
                   } && requires(T wrapper, Atom atom) {
                            {
                                wrapper.atomName(atom)
                            } -> std::convertible_to<std::unique_ptr<char[], decltype(&XFree)>>;
                        } && requires(T wrapper) {
                                 { wrapper.getActiveWindow() } -> std::convertible_to<std::optional<Window>>;
                             } && requires(T wrapper) {
                                      { wrapper.getFocusWindow() } -> std::convertible_to<std::optional<Window>>;
                                  } && requires(T wrapper, Window window) {
                                           {
                                               wrapper.getWindowName(window)
                                           } -> std::convertible_to<std::optional<std::string>>;
                                       } && requires(T wrapper, Window window) {
                                                {
                                                    wrapper.getWindowSize(window)
                                                } -> std::convertible_to<std::optional<XWindowDimensions>>;
                                            } && requires(T wrapper, Window window) {
                                                     {
                                                         wrapper.getWindowPosition(window)
                                                     } -> std::convertible_to<std::optional<XWindowDimensions>>;
                                                 } && requires(T wrapper) {
                                                          { wrapper.nextEvent() } -> std::convertible_to<XEvent>;
                                                      } && requires(T wrapper, XEvent& event) {
                                                               {
                                                                   wrapper.eventData(event)
                                                               } -> std::convertible_to<XEventPointer>;
                                                           } && requires(T wrapper, int& major, int& minor) {
                                                                    {
                                                                        wrapper.queryVersion(major, minor)
                                                                    } -> std::convertible_to<Status>;
                                                                } && requires(T wrapper, XIEventMask& mask) {
                                                                         wrapper.selectEvents(mask);
                                                                     };
}  // namespace EvgetX11

#endif  // EVGET_EVGETX11_INCLUDE_EVGETX11_XWRAPPER_H
