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

#include "evgetx11/XWrapperX11.h"

#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <spdlog/spdlog.h>

#include <array>

EvgetX11::XWrapperX11::XWrapperX11(Display& display) : display{display} {}

std::string EvgetX11::XWrapperX11::lookupCharacter(const XIDeviceEvent& event, KeySym& keySym) {
    if (event.evtype == XI_KeyPress) {
        // Converts XIDeviceEvent to a XKeyEvent in order to leverage existing functions for determining KeySyms. Seems
        // a little bit hacky to do this conversion, however it should be okay as all the elements have a direct
        // relationship.
        XKeyEvent keyEvent{
            .type = KeyPress,
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
            .same_screen = true};

        int bytes;
        std::array<char, utf8MaxBytes + 1> array{};
        if (xic) {
            Status status;
            bytes = Xutf8LookupString(xic.get(), &keyEvent, array.data(), utf8MaxBytes, &keySym, &status);
            if (status != Success || status == XBufferOverflow || bytes == 0) {
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

std::unique_ptr<_XIC, decltype(&XDestroyIC)> EvgetX11::XWrapperX11::createIC(Display& display, XIM xim) {
    if (xim) {
        XIMStyles *styles_ptr;
        auto values = XGetIMValues(xim, XNQueryInputStyle, &styles_ptr, nullptr);
        auto xim_styles = std::unique_ptr<XIMStyles, decltype(&XFree)>{styles_ptr, XFree};

        if (values || !xim_styles) {
            spdlog::warn(
                "The input method does not support any styles, falling back to encoding key events in ISO Latin-1."
            );
            return {nullptr, XDestroyIC};
        }

        for (int i = 0; i < xim_styles->count_styles; i++) {
            if (xim_styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) {
                Window window = XDefaultRootWindow(&display);
                return {
                    XCreateIC(
                        xim,
                        XNInputStyle,
                        XIMPreeditNothing | XIMStatusNothing,
                        XNClientWindow,
                        window,
                        XNFocusWindow,
                        window,
                        nullptr
                    ),
                    XDestroyIC};
            }
        }

        spdlog::warn(
            "The input method does not support the PreeditNothing and StatusNothing style, falling back to encoding "
            "key events in ISO Latin-1."
        );
    }
    return {nullptr, XDestroyIC};
}

std::unique_ptr<unsigned char[]> EvgetX11::XWrapperX11::getDeviceButtonMapping(int id, int mapSize) {
    auto device = std::unique_ptr<XDevice, DeleterWithDisplay<XCloseDevice>>(
        XOpenDevice(&display.get(), id),
        DeleterWithDisplay<XCloseDevice>{display.get()}
    );
    if (device) {
        auto map = std::make_unique<unsigned char[]>(mapSize);
        XGetDeviceButtonMapping(&display.get(), device.get(), map.get(), mapSize);
        return map;
    }
    return nullptr;
}

std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> EvgetX11::XWrapperX11::listInputDevices(int& nDevices) {
    return {XListInputDevices(&display.get(), &nDevices), XFreeDeviceList};
}

std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> EvgetX11::XWrapperX11::queryDevice(int& nDevices) {
    return {XIQueryDevice(&display.get(), XIAllDevices, &nDevices), XIFreeDeviceInfo};
}

std::unique_ptr<char[], decltype(&XFree)> EvgetX11::XWrapperX11::atomName(Atom atom) {
    return {XGetAtomName(&display.get(), atom), XFree};
}

std::optional<Atom> EvgetX11::XWrapperX11::getAtom(const char* atomName) {
    Atom atom = XInternAtom(&display.get(), atomName, true);
    if (atom == None) {
        spdlog::warn("failed to get {} atom");
        return std::nullopt;
    }
    return atom;
}

XEvent EvgetX11::XWrapperX11::nextEvent() {
    XEvent event;
    XNextEvent(&display.get(), &event);
    return event;
}

EvgetX11::XEventPointer EvgetX11::XWrapperX11::eventData(XEvent& event) {
    auto deleter = std::function<void(XGenericEventCookie*)>{DeleterWithDisplay<XFreeEventData>{display.get()}};
    if (XGetEventData(&display.get(), &event.xcookie) && (&event.xcookie)->type == GenericEvent) {
        spdlog::trace(fmt::format("Event type {} captured.", (&event.xcookie)->type));
        return {&event.xcookie, std::move(deleter)};
    }
    return {nullptr, [](XGenericEventCookie*) {}};
}

Status EvgetX11::XWrapperX11::queryVersion(int& major, int& minor) {
    return XIQueryVersion(&display.get(), &major, &minor);
}

void EvgetX11::XWrapperX11::selectEvents(XIEventMask& mask) {
    XISelectEvents(&display.get(), XDefaultRootWindow(&display.get()), &mask, 1);
    XSync(&display.get(), false);
}

std::unique_ptr<unsigned char[], decltype(&XFree)>
EvgetX11::XWrapperX11::getProperty(Atom atom, Window window, unsigned long& nItems, Atom& type, int& size) {
    unsigned char* data;
    unsigned long _bytesAfter;
    Status status = XGetWindowProperty(
        &display.get(),
        window,
        atom,
        0,
        LONG_MAX,
        False,
        AnyPropertyType,
        &type,
        &size,
        &nItems,
        &_bytesAfter,
        &data
    );

    auto prop = std::unique_ptr<unsigned char[], decltype(&XFree)>{data, XFree};

    if (status == BadWindow) {
        spdlog::warn("window does not exist");
        return {nullptr, XFree};
    }
    if (status != Success) {
        spdlog::warn("failed to get window property");
        return {nullptr, XFree};
    }

     return prop;
}

std::optional<std::string> EvgetX11::XWrapperX11::getWindowName(Window window) {
    auto wmNameAtom = getAtom("_NET_WM_NAME");
    if (!wmNameAtom) {
        return std::nullopt;
    }

    Atom type;
    int size;
    unsigned long nItems;

    auto name = getProperty(*wmNameAtom, window, nItems, type, size);
    if (nItems == 0) {
        wmNameAtom = getAtom("WM_NAME");
        if (!wmNameAtom) {
            return std::nullopt;
        }
        name = getProperty(*wmNameAtom, window, nItems, type, size);
    }

    return std::string{reinterpret_cast<const char*>(name.get()), nItems};
}

std::optional<Window> EvgetX11::XWrapperX11::getActiveWindow() {
    auto activeWindow = getAtom("_NET_ACTIVE_WINDOW");
    if (!activeWindow) {
        return std::nullopt;
    }

    Atom type;
    int size;
    unsigned long nItems;

    auto window = getProperty(*activeWindow, XDefaultRootWindow(&display.get()), nItems, type, size);
    if (nItems > 0 && size == 32 && window != nullptr) {
        return *(reinterpret_cast<Window*>(window.get()));
    }

    spdlog::warn("failed to get active window");
    return std::nullopt;
}

std::optional<Window> EvgetX11::XWrapperX11::getFocusWindow() {
    Window window;
    int _revert;

    Status status = XGetInputFocus(&display.get(), &window, &_revert);
    if (status != Success) {
        spdlog::warn("failed to get focus window");
        return std::nullopt;
    }

    return window;
}

std::optional<XWindowAttributes> EvgetX11::XWrapperX11::getWindowAttributes(Window window) {
    XWindowAttributes attributes;

    // XGetWindowAttributes returns non-zero on success.
    if (!XGetWindowAttributes(&display.get(), window, &attributes)) {
        spdlog::warn("failed to get window attributes");
        return std::nullopt;
    }

    return attributes;
}

std::optional<EvgetX11::XWindowDimensions> EvgetX11::XWrapperX11::getWindowSize(Window window) {
    auto attributes = getWindowAttributes(window);

    if (!attributes.has_value()) {
        return std::nullopt;
    }

    unsigned int width = attributes->width;
    unsigned int height = attributes->width;

    return {{width, height}};
}

std::optional<EvgetX11::XWindowDimensions> EvgetX11::XWrapperX11::getWindowPosition(Window window) {
    auto attributes = getWindowAttributes(window);

    if (!attributes.has_value()) {
        return std::nullopt;
    }

    Window parent, root;
    unsigned int nChildren;
    Window* childrenReturn;

    XQueryTree(&display.get(), window, &root, &parent, &childrenReturn, &nChildren);
    auto children = std::unique_ptr<Window[], decltype(&XFree)>{childrenReturn, XFree};

    // It shouldn't be necessary to check if the parent is the root, but it shouldn't hurt.
    // See https://github.com/jordansissel/xdotool/pull/9 for more information.
    int x, y;
    if (parent == attributes->root) {
        x = attributes->x;
        y = attributes->y;
    } else {
        Window _unused;
        XTranslateCoordinates(&display.get(), window, attributes->root, 0, 0, &x, &y, &_unused);
    }

    unsigned int width = x;
    unsigned int height = y;

    return {{width, height}};
}

void EvgetX11::XWrapperX11::setMask(unsigned char* mask, std::initializer_list<int> events) {
    for (auto event : events) {
        XISetMask(mask, event);
    }
}

std::string EvgetX11::XWrapperX11::keySymToString(KeySym keySym) {
    if (keySym != NoSymbol) {
        auto name = XKeysymToString(keySym);
        if (name != nullptr) {
            return {name};
        }
    }
    return "";
}
