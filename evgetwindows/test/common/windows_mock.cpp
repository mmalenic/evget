#include "common/windows_mock.h"

#include "evgetwindows/raw_event.h"

namespace test {

evgetwindows::RawEvent MakeMouseRawEvent() {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEMOUSE;
    event.data.mouse.lLastX = 0x1234;
    event.data.mouse.lLastY = 0x5678;
    return event;
}

evgetwindows::RawEvent MakeKeyboardRawEvent() {
    evgetwindows::RawEvent event{};
    event.header.dwType = RIM_TYPEKEYBOARD;
    event.data.keyboard.VKey = static_cast<USHORT>(0x41);
    return event;
}

} // namespace test
