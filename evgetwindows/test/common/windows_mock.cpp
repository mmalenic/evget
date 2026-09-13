#include "common/windows_mock.h"

#include "evgetwindows/raw_event.h"

namespace test {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-union-access)

evgetwindows::RawEvent MakeMouseRawEvent() {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.lLastX = 0x1234;
    mouse.lLastY = 0x5678;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeKeyboardRawEvent() {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEKEYBOARD;
    RAWKEYBOARD keyboard{};
    keyboard.VKey = static_cast<USHORT>(0x41);
    event.data = keyboard;
    return event;
}

evgetwindows::RawEvent MakeMouseMoveRelative(LONG last_x, LONG last_y) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usFlags = MOUSE_MOVE_RELATIVE;
    mouse.lLastX = last_x;
    mouse.lLastY = last_y;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeMouseWheel(SHORT delta, bool horizontal) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usButtonFlags = static_cast<USHORT>(horizontal ? RI_MOUSE_HWHEEL : RI_MOUSE_WHEEL);
    mouse.usButtonData = static_cast<USHORT>(delta);
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeMouseButton(USHORT button_flags) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usButtonFlags = button_flags;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeMouseAbsolute(LONG last_x, LONG last_y) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    RAWMOUSE mouse{};
    mouse.usFlags = MOUSE_MOVE_ABSOLUTE;
    mouse.lLastX = last_x;
    mouse.lLastY = last_y;
    event.data = mouse;
    return event;
}

evgetwindows::RawEvent MakeKeyboard(USHORT vkey, USHORT make_code, USHORT flags) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEKEYBOARD;
    RAWKEYBOARD keyboard{};
    keyboard.VKey = vkey;
    keyboard.MakeCode = make_code;
    keyboard.Flags = flags;
    event.data = keyboard;
    return event;
}

evgetwindows::RawEvent MakeInjected(USHORT vkey) {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEKEYBOARD;
    event.header.hDevice = nullptr;
    RAWKEYBOARD keyboard{};
    keyboard.VKey = vkey;
    event.data = keyboard;
    return event;
}

RAWINPUT MakeMouseRawInput(LONG last_x, LONG last_y) {
    RAWINPUT raw{};
    raw.header.dwType = RIM_TYPEMOUSE;
    raw.data.mouse.lLastX = last_x;
    raw.data.mouse.lLastY = last_y;
    return raw;
}

RAWINPUT MakeKeyboardRawInput(USHORT vkey) {
    RAWINPUT raw{};
    raw.header.dwType = RIM_TYPEKEYBOARD;
    raw.data.keyboard.VKey = vkey;
    return raw;
}

RAWINPUT MakeHidRawInput() {
    RAWINPUT raw{};
    raw.header.dwType = RIM_TYPEHID;
    return raw;
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-union-access)

} // namespace test
