#ifndef EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H
#define EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H

#include <gmock/gmock.h>

#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows.h"

namespace test {

class WindowsApiMock : public evgetwindows::WindowsApi {
public:
    MOCK_METHOD(evget::Result<void>, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD((boost::asio::awaitable<evget::Result<evgetwindows::RawEvent>>), ReceiveNext, (), (override));
};

evgetwindows::RawEvent MakeMouseRawEvent();
evgetwindows::RawEvent MakeKeyboardRawEvent();

RAWINPUT MakeMouseRawInput(LONG last_x, LONG last_y);
RAWINPUT MakeKeyboardRawInput(USHORT vkey);
RAWINPUT MakeHidRawInput();

} // namespace test

#endif // EVGETWINDOWS_TEST_COMMON_WINDOWS_MOCK_H
