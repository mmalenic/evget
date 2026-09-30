#include "evgetx11/x11.h"

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>
#include <spdlog/spdlog.h>
#include <xorg/xserver-properties.h>

#include <array>
#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace {
constexpr int kUtf8MaxBytes = 4;
constexpr int kWindowPropertySize = 32;

constexpr unsigned char kAsciiEnd = 0x80;
constexpr unsigned char kUtf8TwoByteLead = 0xC0;
constexpr unsigned char kUtf8Continuation = 0x80;
constexpr unsigned char kUtf8ContinuationMask = 0x3F;
constexpr int kUtf8ContinuationBits = 6;
} // namespace

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays,
// cppcoreguidelines-pro-type-vararg, hicpp-vararg)
evgetx11::X11::X11(Display& display) : display_{display} {}

std::string
evgetx11::X11::LookupCharacter(const XIRawEvent& event, const QueryPointerResult& query_pointer, KeySym& key_sym) {
    if (event.evtype == XI_RawKeyPress || event.evtype == XI_RawKeyRelease) {
        // Converts XIDeviceEvent to a XKeyEvent in order to leverage existing functions for determining KeySyms. Seems
        // a little bit hacky to do this conversion, however it should be okay as all the elements have a direct
        // relationship.
        XKeyEvent key_event{
            .type = event.evtype == XI_RawKeyPress ? KeyPress : KeyRelease,
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

        std::array<char, kUtf8MaxBytes + 1> latin1{};
        const int latin1_bytes = XLookupString(&key_event, latin1.data(), kUtf8MaxBytes, &key_sym, nullptr);
        if (event.evtype == XI_RawKeyRelease) {
            // Release has a key sym but no character, so we fetch it anyway before returning.
            return {};
        }

        if (xic_) {
            std::array<char, kUtf8MaxBytes + 1> utf8{};
            KeySym composed = NoSymbol;
            Status status = 0;
            const int bytes = Xutf8LookupString(xic_.get(), &key_event, utf8.data(), kUtf8MaxBytes, &composed, &status);
            if ((status == XLookupChars || status == XLookupBoth) && bytes > 0) {
                return std::string{utf8.data(), static_cast<std::size_t>(bytes)};
            }
            spdlog::debug("Xutf8LookupString did not return a value, falling back to XLookupString");
        }
        return Latin1ToUtf8({latin1.data(), static_cast<std::size_t>(latin1_bytes)});
    }
    return {};
}

std::unique_ptr<_XIC, decltype(&XDestroyIC)> evgetx11::X11::CreateIc(Display& display, XIM xim) {
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

std::unique_ptr<unsigned char[]> evgetx11::X11::GetDeviceButtonMapping(int device_id, int map_size) {
    auto device = std::unique_ptr<XDevice, DisplayDeleter<XCloseDevice>>(
        XOpenDevice(&display_.get(), device_id),
        DisplayDeleter<XCloseDevice>{display_.get()}
    );
    if (device) {
        auto map = std::make_unique<unsigned char[]>(map_size);
        XGetDeviceButtonMapping(&display_.get(), device.get(), map.get(), map_size);
        return map;
    }
    return nullptr;
}

std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> evgetx11::X11::ListInputDevices(int& n_devices) {
    return {XListInputDevices(&display_.get(), &n_devices), XFreeDeviceList};
}

std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> evgetx11::X11::QueryDevice(int& n_devices) {
    return {XIQueryDevice(&display_.get(), XIAllDevices, &n_devices), XIFreeDeviceInfo};
}

std::unique_ptr<char[], decltype(&XFree)> evgetx11::X11::AtomName(Atom atom) {
    return {XGetAtomName(&display_.get(), atom), XFree};
}

std::optional<Atom> evgetx11::X11::GetAtom(const char* atom_name) const {
    Atom atom = XInternAtom(&display_.get(), atom_name, True);
    if (atom == None) {
        log_.Warn(std::format("failed to get {} atom", atom_name));
        return std::nullopt;
    }
    return atom;
}

XEvent evgetx11::X11::NextEvent() {
    XEvent event;
    XNextEvent(&display_.get(), &event);
    return event;
}

evgetx11::XEventPointer evgetx11::X11::EventData(XEvent& event) {
    auto deleter = std::function<void(XGenericEventCookie*)>{DisplayDeleter<XFreeEventData>{display_.get()}};
    if (XGetEventData(&display_.get(), &event.xcookie) != 0 && (&event.xcookie)->type == GenericEvent) {
        spdlog::trace(std::format("Event type {} captured.", (&event.xcookie)->type));
        return {&event.xcookie, std::move(deleter)};
    }
    return {nullptr, [](XGenericEventCookie*) {}};
}

bool evgetx11::X11::IsXTestDevice(int device_id) {
    const Atom xtest = XInternAtom(&display_.get(), XI_PROP_XTEST_DEVICE, True);
    if (xtest == None) {
        return false;
    }

    Atom type = None;
    int format = 0;
    unsigned long n_items = 0;
    unsigned long bytes_after = 0;
    unsigned char* data = nullptr;
    const Status status = XIGetProperty(
        &display_.get(),
        device_id,
        xtest,
        0,
        1,
        False,
        AnyPropertyType,
        &type,
        &format,
        &n_items,
        &bytes_after,
        &data
    );
    const std::unique_ptr<unsigned char[], decltype(&XFree)> property{data, XFree};

    return status == Success && type != None && n_items > 0;
}

int evgetx11::X11::GetConnectionNumber() {
    return XConnectionNumber(&display_.get());
}

int evgetx11::X11::Pending() {
    return XPending(&display_.get());
}

evgetx11::QueryPointerResult evgetx11::X11::QueryPointer(int device_id) {
    Window root_return = 0;
    Window _window_return = 0;
    double _win_x = 0;
    double _win_y = 0;
    double root_x = 0;
    double root_y = 0;
    XIButtonState button_state{};
    XIModifierState modifier_state{};
    XIGroupState group_state{};

    bool found = false;
    for (auto i = 0; i < XScreenCount(&display_.get()); i++) {
        Screen* screen = XScreenOfDisplay(&display_.get(), i);
        auto result = XIQueryPointer(
            &display_.get(),
            device_id,
            XRootWindowOfScreen(screen),
            &root_return,
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
            found = true;
            break;
        }
    }

    std::optional<std::string> screen_name;
    if (found) {
        screen_name = MonitorForPoint(root_return, root_x, root_y);
    }

    return QueryPointerResult{
        .root_x = root_x,
        .root_y = root_y,
        .button_mask = {button_state.mask, XFree},
        .modifier_state = modifier_state,
        .group_state = group_state,
        .screen_name = std::move(screen_name),
    };
}

std::optional<std::string> evgetx11::X11::MonitorForPoint(Window root, double root_x, double root_y) {
    int count = 0;
    const std::unique_ptr<XRRMonitorInfo, decltype(&XRRFreeMonitors)> monitors{
        XRRGetMonitors(&display_.get(), root, True, &count),
        XRRFreeMonitors
    };
    if (monitors == nullptr || count <= 0) {
        log_.Warn("failed to query monitors");
        return std::nullopt;
    }

    const std::span<const XRRMonitorInfo> list{monitors.get(), static_cast<size_t>(count)};
    const auto pointer_x = static_cast<int>(root_x);
    const auto pointer_y = static_cast<int>(root_y);

    for (const XRRMonitorInfo& monitor : list) {
        const bool inside_x = pointer_x >= monitor.x && pointer_x < monitor.x + monitor.width;
        const bool inside_y = pointer_y >= monitor.y && pointer_y < monitor.y + monitor.height;
        if (inside_x && inside_y) {
            auto name = AtomName(monitor.name);
            if (name == nullptr) {
                return std::nullopt;
            }
            return std::string{name.get()};
        }
    }

    return std::nullopt;
}

Status evgetx11::X11::QueryVersion(int& major, int& minor) {
    return XIQueryVersion(&display_.get(), &major, &minor);
}

void evgetx11::X11::SelectEvents(XIEventMask& mask) {
    XISelectEvents(&display_.get(), XDefaultRootWindow(&display_.get()), &mask, 1);
    XSync(&display_.get(), False);
}

evgetx11::X11::GetPropertyResult evgetx11::X11::GetProperty(Atom atom, Window window) const {
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
        log_.Warn("window does not exist");
        return {.property = {nullptr, XFree}};
    }
    if (status != Success) {
        log_.Warn("failed to get window property");
        return {.property = {nullptr, XFree}};
    }

    return {.n_items = n_items, .type = type, .size = size, .property = std::move(prop)};
}

std::optional<std::string> evgetx11::X11::GetWindowName(Window window) {
    GetPropertyResult name{.property = {nullptr, XFree}};
    if (auto net_wm_name = GetAtom("_NET_WM_NAME"); net_wm_name.has_value()) {
        name = GetProperty(*net_wm_name, window);
    }
    if (name.n_items == 0) {
        name = GetProperty(XA_WM_NAME, window);
    }

    if (name.property == nullptr) {
        return std::string{};
    }

    // Reinterpret cast should be safe converting unsigned char to char.
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    const std::string_view text{reinterpret_cast<const char*>(name.property.get()), name.n_items};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    if (name.type == XA_STRING) {
        return Latin1ToUtf8(text);
    }
    return std::string{text};
}

std::optional<Window> evgetx11::X11::GetActiveWindow() {
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

    log_.Warn("failed to get active window");
    return std::nullopt;
}

std::optional<Window> evgetx11::X11::GetFocusWindow() {
    Window window = None;
    int _revert = 0;

    // XGetInputFocus has no status, need to check window instead.
    XGetInputFocus(&display_.get(), &window, &_revert);
    if (window == None || window == PointerRoot) {
        return std::nullopt;
    }

    return window;
}

std::optional<XWindowAttributes> evgetx11::X11::GetWindowAttributes(Window window) const {
    XWindowAttributes attributes;

    if (window == 0) {
        return std::nullopt;
    }

    // XGetWindowAttributes returns non-zero on success.
    if (XGetWindowAttributes(&display_.get(), window, &attributes) == 0) {
        log_.Warn("failed to get window attributes");
        return std::nullopt;
    }

    return attributes;
}

std::optional<evgetx11::XWindowDimensions> evgetx11::X11::GetWindowSize(Window window) {
    auto attributes = GetWindowAttributes(window);

    if (!attributes.has_value()) {
        return std::nullopt;
    }

    const unsigned int width = attributes->width;
    const unsigned int height = attributes->height;

    return {{.width = width, .height = height}};
}

std::optional<evgetx11::XWindowDimensions> evgetx11::X11::GetWindowPosition(Window window) {
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

void evgetx11::X11::SetMask(unsigned char* mask, std::initializer_list<int> events) {
    for (auto event : events) {
        XISetMask(mask, event);
    }
}

std::string evgetx11::X11::Latin1ToUtf8(std::string_view latin1) {
    std::string utf8{};
    for (const char byte : latin1) {
        const auto code_point = static_cast<unsigned char>(byte);
        if (code_point < kAsciiEnd) {
            utf8.push_back(byte);
            continue;
        }
        utf8.push_back(static_cast<char>(kUtf8TwoByteLead | (code_point >> kUtf8ContinuationBits)));
        utf8.push_back(static_cast<char>(kUtf8Continuation | (code_point & kUtf8ContinuationMask)));
    }
    return utf8;
}

std::string evgetx11::X11::KeySymToString(KeySym key_sym) {
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
