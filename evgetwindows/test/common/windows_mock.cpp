#include "common/windows_mock.h"

#include "evgetwindows/raw_event.h"

namespace test {

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

} // namespace test
