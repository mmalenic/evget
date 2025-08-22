#include "evgetx11/XWrapperX11.h"

#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <spdlog/spdlog.h>

#include <array>
#include <format>

#include "evgetx11/DeleterWithDisplay.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays,
// cppcoreguidelines-pro-type-vararg, hicpp-vararg)
evgetx11::XWrapperX11::XWrapperX11(Display& display) : display_{display} {}

std::string evgetx11::XWrapperX11::LookupCharacter(
    const XIRawEvent& event,
    const QueryPointerResult& query_pointer,
    KeySym& key_sym
) {
    if (event.evtype == XI_RawKeyPress) {
        // Converts XIDeviceEvent to a XKeyEvent in order to leverage existing functions for determining KeySyms. Seems
        // a little bit hacky to do this conversion, however it should be okay as all the elements have a direct
        // relationship.
        XKeyEvent key_event{
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
        std::array<char, kUtf8MaxBytes + 1> array{};
        if (xic_) {
            Status status = 0;
            bytes = Xutf8LookupString(xic_.get(), &key_event, array.data(), kUtf8MaxBytes, &key_sym, &status);
            if (status != Success || status == XBufferOverflow || bytes == 0) {
                spdlog::info("Xutf8LookupString did not return a value, falling back on XLookupString");
                bytes = XLookupString(&key_event, array.data(), kUtf8MaxBytes, &key_sym, nullptr);
            }
        } else {
            bytes = XLookupString(&key_event, array.data(), kUtf8MaxBytes, &key_sym, nullptr);
        }

        array.at(bytes) = '\0';
        return std::string{array.data()};
    }
    return {};
}

std::unique_ptr<_XIC, decltype(&XDestroyIC)> evgetx11::XWrapperX11::CreateIc(Display& display, XIM xim) {
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

std::unique_ptr<unsigned char[]> evgetx11::XWrapperX11::GetDeviceButtonMapping(int device_id, int map_size) {
    auto device = std::unique_ptr<XDevice, DeleterWithDisplay<XCloseDevice>>(
        XOpenDevice(&display_.get(), device_id),
        DeleterWithDisplay<XCloseDevice>{display_.get()}
    );
    if (device) {
        auto map = std::make_unique<unsigned char[]>(map_size);
        XGetDeviceButtonMapping(&display_.get(), device.get(), map.get(), map_size);
        return map;
    }
    return nullptr;
}

std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> evgetx11::XWrapperX11::ListInputDevices(int& n_devices) {
    return {XListInputDevices(&display_.get(), &n_devices), XFreeDeviceList};
}

std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> evgetx11::XWrapperX11::QueryDevice(int& n_devices) {
    return {XIQueryDevice(&display_.get(), XIAllDevices, &n_devices), XIFreeDeviceInfo};
}

std::unique_ptr<char[], decltype(&XFree)> evgetx11::XWrapperX11::AtomName(Atom atom) {
    return {XGetAtomName(&display_.get(), atom), XFree};
}

std::optional<Atom> evgetx11::XWrapperX11::GetAtom(const char* atom_name) const {
    Atom atom = XInternAtom(&display_.get(), atom_name, True);
    if (atom == None) {
        spdlog::warn("failed to get {} atom");
        return std::nullopt;
    }
    return atom;
}

XEvent evgetx11::XWrapperX11::NextEvent() {
    XEvent event;
    XNextEvent(&display_.get(), &event);
    return event;
}

evgetx11::XEventPointer evgetx11::XWrapperX11::EventData(XEvent& event) {
    auto deleter = std::function<void(XGenericEventCookie*)>{DeleterWithDisplay<XFreeEventData>{display_.get()}};
    if (XGetEventData(&display_.get(), &event.xcookie) != 0 && (&event.xcookie)->type == GenericEvent) {
        spdlog::trace(std::format("Event type {} captured.", (&event.xcookie)->type));
        return {&event.xcookie, std::move(deleter)};
    }
    return {nullptr, [](XGenericEventCookie*) {}};
}

evgetx11::QueryPointerResult evgetx11::XWrapperX11::QueryPointer(int device_id) {
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
    for (auto i = 0; i < XScreenCount(&display_.get()); i++) {
        Screen* screen = XScreenOfDisplay(&display_.get(), i);
        auto result = XIQueryPointer(
            &display_.get(),
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

Status evgetx11::XWrapperX11::QueryVersion(int& major, int& minor) {
    return XIQueryVersion(&display_.get(), &major, &minor);
}

void evgetx11::XWrapperX11::SelectEvents(XIEventMask& mask) {
    XISelectEvents(&display_.get(), XDefaultRootWindow(&display_.get()), &mask, 1);
    XSync(&display_.get(), False);
}

evgetx11::XWrapperX11::GetPropertyResult evgetx11::XWrapperX11::GetProperty(Atom atom, Window window) const {
    unsigned char* data = nullptr;
    unsigned long _bytes_after = 0;

    if (window == 0) {
        return {.property = {nullptr, XFree}};
    }

    unsigned long n_items = 0;
    Atom type = 0;
    int size = 0;
    Status status = XGetWindowProperty(
        &display_.get(),
        window,
        atom,
        0,
        LONG_MAX,
        False,
        AnyPropertyType,
        &type,
        &size,
        &n_items,
        &_bytes_after,
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

    return {.n_items = n_items, .type = type, .size = size, .property = std::move(prop)};
}

std::optional<std::string> evgetx11::XWrapperX11::GetWindowName(Window window) {
    auto wm_name_atom = GetAtom("_NET_WM_NAME");
    if (!wm_name_atom) {
        return std::nullopt;
    }

    auto name = GetProperty(*wm_name_atom, window);
    if (name.n_items == 0) {
        wm_name_atom = GetAtom("WM_NAME");
        if (!wm_name_atom) {
            return std::nullopt;
        }
        name = GetProperty(*wm_name_atom, window);
    }

    if (name.property == nullptr) {
        return std::string{};
    }

    // Reinterpret cast should be safe converting unsigned char to char.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::string{reinterpret_cast<const char*>(name.property.get()), name.n_items};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}

std::optional<Window> evgetx11::XWrapperX11::GetActiveWindow() {
    auto active_window = GetAtom("_NET_ACTIVE_WINDOW");
    if (!active_window) {
        return std::nullopt;
    }

    auto window = GetProperty(*active_window, XDefaultRootWindow(&display_.get()));
    if (window.n_items > 0 && window.size == kWindowPropertySize && window.property != nullptr) {
        // Reinterpret cast is required by X11.
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<const Window*>(window.property.get());
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    spdlog::warn("failed to get active window");
    return std::nullopt;
}

std::optional<Window> evgetx11::XWrapperX11::GetFocusWindow() {
    Window window = 0;
    int _revert = 0;

    Status status = XGetInputFocus(&display_.get(), &window, &_revert);
    if (status != Success) {
        spdlog::warn("failed to get focus window");
        return std::nullopt;
    }

    return window;
}

std::optional<XWindowAttributes> evgetx11::XWrapperX11::GetWindowAttributes(Window window) const {
    XWindowAttributes attributes;

    if (window == 0) {
        return std::nullopt;
    }

    // XGetWindowAttributes returns non-zero on success.
    if (XGetWindowAttributes(&display_.get(), window, &attributes) == 0) {
        spdlog::warn("failed to get window attributes");
        return std::nullopt;
    }

    return attributes;
}

std::optional<evgetx11::XWindowDimensions> evgetx11::XWrapperX11::GetWindowSize(Window window) {
    auto attributes = GetWindowAttributes(window);

    if (!attributes.has_value()) {
        return std::nullopt;
    }

    const unsigned int width = attributes->width;
    const unsigned int height = attributes->width;

    return {{.width = width, .height = height}};
}

std::optional<evgetx11::XWindowDimensions> evgetx11::XWrapperX11::GetWindowPosition(Window window) {
    auto attributes = GetWindowAttributes(window);

    if (!attributes.has_value()) {
        return std::nullopt;
    }

    Window parent = 0;
    Window root = 0;
    unsigned int n_children = 0;
    Window* children_return = nullptr;

    XQueryTree(&display_.get(), window, &root, &parent, &children_return, &n_children);
    auto children = std::unique_ptr<Window[], decltype(&XFree)>{children_return, XFree};

    // It shouldn't be necessary to check if the parent is the root, but it shouldn't hurt.
    // See https://github.com/jordansissel/xdotool/pull/9 for more information.
    int x_pos = 0;
    int y_pos = 0;
    if (parent == attributes->root) {
        x_pos = attributes->x;
        y_pos = attributes->y;
    } else {
        Window _unused = 0;
        XTranslateCoordinates(&display_.get(), window, attributes->root, 0, 0, &x_pos, &y_pos, &_unused);
    }

    const unsigned int width = x_pos;
    const unsigned int height = y_pos;

    return {{.width = width, .height = height}};
}

void evgetx11::XWrapperX11::SetMask(unsigned char* mask, std::initializer_list<int> events) {
    for (auto event : events) {
        XISetMask(mask, event);
    }
}

std::string evgetx11::XWrapperX11::KeySymToString(KeySym key_sym) {
    if (key_sym != NoSymbol) {
        auto* name = XKeysymToString(key_sym);
        if (name != nullptr) {
            return {name};
        }
    }
    return "";
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays,
// cppcoreguidelines-pro-type-vararg, hicpp-vararg)
