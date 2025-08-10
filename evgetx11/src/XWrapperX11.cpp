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

#include "evgetx11/DeleterWithDisplay.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays,
// cppcoreguidelines-pro-type-vararg, hicpp-vararg)
EvgetX11::XWrapperX11::XWrapperX11(Display& display) : display{display} {}

std::string EvgetX11::XWrapperX11::lookupCharacter(
    const XIRawEvent& event,
    const QueryPointerResult& query_pointer,
    KeySym& keySym
) {
    if (event.evtype == XI_RawKeyPress) {
        // Converts XIDeviceEvent to a XKeyEvent in order to leverage existing functions for determining KeySyms. Seems
        // a little bit hacky to do this conversion, however it should be okay as all the elements have a direct
        // relationship.
        XKeyEvent keyEvent{
            .type = KeyPress,
            .serial = event.serial,
            .send_event = event.send_event,
            .display = event.display,
            .window = 0,
            .root = 0,
            .subwindow = 0,
            .time = event.time,
            .x = static_cast<int>(query_pointer.root_x),
            .y = static_cast<int>(query_pointer.root_y),
            .x_root = static_cast<int>(query_pointer.root_x),
            .y_root = static_cast<int>(query_pointer.root_y),
            .state = static_cast<unsigned int>(query_pointer.modifier_state.effective),
            .keycode = static_cast<unsigned int>(event.detail),
            .same_screen = 1
        };

        int bytes = 0;
        std::array<char, utf8MaxBytes + 1> array{};
        if (xic) {
            Status status = 0;
            bytes = Xutf8LookupString(xic.get(), &keyEvent, array.data(), utf8MaxBytes, &keySym, &status);
            if (status != Success || status == XBufferOverflow || bytes == 0) {
                spdlog::info("Xutf8LookupString did not return a value, falling back on XLookupString");
                bytes = XLookupString(&keyEvent, array.data(), utf8MaxBytes, &keySym, nullptr);
            }
        } else {
            bytes = XLookupString(&keyEvent, array.data(), utf8MaxBytes, &keySym, nullptr);
        }

        array.at(bytes) = '\0';
        return std::string{array.data()};
    }
    return {};
}

std::unique_ptr<_XIC, decltype(&XDestroyIC)> EvgetX11::XWrapperX11::createIC(Display& display, XIM xim) {
    if (xim != nullptr) {
        XIMStyles* styles_ptr = nullptr;
        auto* values = XGetIMValues(xim, XNQueryInputStyle, &styles_ptr, nullptr);
        auto xim_styles = std::unique_ptr<XIMStyles, decltype(&XFree)>{styles_ptr, XFree};

        if (values != nullptr || !xim_styles) {
            spdlog::warn(
                "The input method does not support any styles, falling back to encoding key events in ISO Latin-1."
            );
            return {nullptr, XDestroyIC};
        }

        for (std::uint16_t i = 0; i < xim_styles->count_styles; i++) {
            if (xim_styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing)) {
                const Window window = XDefaultRootWindow(&display);
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
                    XDestroyIC
                };
            }
        }

        spdlog::warn(
            "The input method does not support the PreeditNothing and StatusNothing style, falling back to encoding "
            "key events in ISO Latin-1."
        );
    }
    return {nullptr, XDestroyIC};
}

std::unique_ptr<unsigned char[]> EvgetX11::XWrapperX11::getDeviceButtonMapping(int device_id, int mapSize) {
    auto device = std::unique_ptr<XDevice, DeleterWithDisplay<XCloseDevice>>(
        XOpenDevice(&display.get(), device_id),
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

std::optional<Atom> EvgetX11::XWrapperX11::getAtom(const char* atomName) const {
    Atom atom = XInternAtom(&display.get(), atomName, True);
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
    if (XGetEventData(&display.get(), &event.xcookie) != 0 && (&event.xcookie)->type == GenericEvent) {
        spdlog::trace(fmt::format("Event type {} captured.", (&event.xcookie)->type));
        return {&event.xcookie, std::move(deleter)};
    }
    return {nullptr, [](XGenericEventCookie*) {}};
}

EvgetX11::QueryPointerResult EvgetX11::XWrapperX11::query_pointer(int device_id) {
    Window _root_return = 0;
    Window _window_return = 0;
    double _win_x = 0;
    double _win_y = 0;
    double root_x = 0;
    double root_y = 0;
    XIButtonState button_state;
    XIModifierState modifier_state;
    XIGroupState group_state;

    auto screen_number = 0;
    for (auto i = 0; i < XScreenCount(&display.get()); i++) {
        Screen* screen = XScreenOfDisplay(&display.get(), i);
        auto result = XIQueryPointer(
            &display.get(),
            device_id,
            XRootWindowOfScreen(screen),
            &_root_return,
            &_window_return,
            &root_x,
            &root_y,
            &_win_x,
            &_win_y,
            &button_state,
            &modifier_state,
            &group_state
        );
        if (result == True) {
            screen_number = i;
            break;
        }
    }

    return QueryPointerResult{
        .root_x = root_x,
        .root_y = root_y,
        .button_mask = {button_state.mask, XFree},
        .modifier_state = modifier_state,
        .group_state = group_state,
        .screen_number = screen_number,
    };
}

Status EvgetX11::XWrapperX11::queryVersion(int& major, int& minor) {
    return XIQueryVersion(&display.get(), &major, &minor);
}

void EvgetX11::XWrapperX11::selectEvents(XIEventMask& mask) {
    XISelectEvents(&display.get(), XDefaultRootWindow(&display.get()), &mask, 1);
    XSync(&display.get(), False);
}

EvgetX11::XWrapperX11::GetPropertyResult EvgetX11::XWrapperX11::getProperty(Atom atom, Window window) const {
    unsigned char* data = nullptr;
    unsigned long _bytesAfter = 0;

    if (window == 0) {
        return {.property = {nullptr, XFree}};
    }

    unsigned long nItems = 0;
    Atom type = 0;
    int size = 0;
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
        return {.property = {nullptr, XFree}};
    }
    if (status != Success) {
        spdlog::warn("failed to get window property");
        return {.property = {nullptr, XFree}};
    }

    return {.nItems = nItems, .type = type, .size = size, .property = std::move(prop)};
}

std::optional<std::string> EvgetX11::XWrapperX11::getWindowName(Window window) {
    auto wmNameAtom = getAtom("_NET_WM_NAME");
    if (!wmNameAtom) {
        return std::nullopt;
    }

    auto name = getProperty(*wmNameAtom, window);
    if (name.nItems == 0) {
        wmNameAtom = getAtom("WM_NAME");
        if (!wmNameAtom) {
            return std::nullopt;
        }
        name = getProperty(*wmNameAtom, window);
    }

    if (name.property == nullptr) {
        return std::string{};
    }

    // Reinterpret cast should be safe converting unsigned char to char.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::string{reinterpret_cast<const char*>(name.property.get()), name.nItems};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}

std::optional<Window> EvgetX11::XWrapperX11::getActiveWindow() {
    auto activeWindow = getAtom("_NET_ACTIVE_WINDOW");
    if (!activeWindow) {
        return std::nullopt;
    }

    auto window = getProperty(*activeWindow, XDefaultRootWindow(&display.get()));
    if (window.nItems > 0 && window.size == windowPropertySize && window.property != nullptr) {
        // Reinterpret cast is required by X11.
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<const Window*>(window.property.get());
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    spdlog::warn("failed to get active window");
    return std::nullopt;
}

std::optional<Window> EvgetX11::XWrapperX11::getFocusWindow() {
    Window window = 0;
    int _revert = 0;

    Status status = XGetInputFocus(&display.get(), &window, &_revert);
    if (status != Success) {
        spdlog::warn("failed to get focus window");
        return std::nullopt;
    }

    return window;
}

std::optional<XWindowAttributes> EvgetX11::XWrapperX11::getWindowAttributes(Window window) const {
    XWindowAttributes attributes;

    if (window == 0) {
        return std::nullopt;
    }

    // XGetWindowAttributes returns non-zero on success.
    if (XGetWindowAttributes(&display.get(), window, &attributes) == 0) {
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

    const unsigned int width = attributes->width;
    const unsigned int height = attributes->width;

    return {{.width = width, .height = height}};
}

std::optional<EvgetX11::XWindowDimensions> EvgetX11::XWrapperX11::getWindowPosition(Window window) {
    auto attributes = getWindowAttributes(window);

    if (!attributes.has_value()) {
        return std::nullopt;
    }

    Window parent = 0;
    Window root = 0;
    unsigned int nChildren = 0;
    Window* childrenReturn = nullptr;

    XQueryTree(&display.get(), window, &root, &parent, &childrenReturn, &nChildren);
    auto children = std::unique_ptr<Window[], decltype(&XFree)>{childrenReturn, XFree};

    // It shouldn't be necessary to check if the parent is the root, but it shouldn't hurt.
    // See https://github.com/jordansissel/xdotool/pull/9 for more information.
    int x_pos = 0;
    int y_pos = 0;
    if (parent == attributes->root) {
        x_pos = attributes->x;
        y_pos = attributes->y;
    } else {
        Window _unused = 0;
        XTranslateCoordinates(&display.get(), window, attributes->root, 0, 0, &x_pos, &y_pos, &_unused);
    }

    const unsigned int width = x_pos;
    const unsigned int height = y_pos;

    return {{.width = width, .height = height}};
}

void EvgetX11::XWrapperX11::setMask(unsigned char* mask, std::initializer_list<int> events) {
    for (auto event : events) {
        XISetMask(mask, event);
    }
}

std::string EvgetX11::XWrapperX11::keySymToString(KeySym keySym) {
    if (keySym != NoSymbol) {
        auto* name = XKeysymToString(keySym);
        if (name != nullptr) {
            return {name};
        }
    }
    return "";
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays,
// cppcoreguidelines-pro-type-vararg, hicpp-vararg)
