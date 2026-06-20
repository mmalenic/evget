#ifndef EVGETLIBINPUT_TEST_COMMON_TEST_HELPERS_H
#define EVGETLIBINPUT_TEST_COMMON_TEST_HELPERS_H

#include <gmock/gmock.h>

#include <libinput.h>
#include <xkbcommon/xkbcommon.h>

#include <cstdint>

#include "evget/error.h"
#include "evgetlibinput/drm.h"
#include "evgetlibinput/libinput.h"
#include "evgetlibinput/xkbcommon.h"

namespace test {

constexpr evgetlibinput::ScreenDimensions kDimensions{.width = 1920, .height = 1080};
constexpr const char* kDeviceName = "test-device";
constexpr xkb_rule_names kUsLayout{
    .rules = nullptr,
    .model = nullptr,
    .layout = "us",
    .variant = nullptr,
    .options = nullptr,
};
constexpr xkb_keycode_t kKeyCodeOffset = 8;

constexpr xkb_keycode_t XkbKey(unsigned int evdev_key) {
    return evdev_key + kKeyCodeOffset;
}

evgetlibinput::XkbCommon MakeUsXkb();

class LibInputApiMock : public evgetlibinput::LibInputApi {
public:
    MOCK_METHOD(evget::Result<evgetlibinput::LibInputEvent>, GetEvent, (), (override));
    MOCK_METHOD(libinput_event_type, GetEventType, (libinput_event & event), (override));
    MOCK_METHOD(libinput_event_pointer*, GetPointerEvent, (libinput_event & event), (override));
    MOCK_METHOD(std::uint64_t, GetPointerTimeMicroseconds, (libinput_event_pointer & event), (override));
    MOCK_METHOD(double, GetPointerDx, (libinput_event_pointer & event), (override));
    MOCK_METHOD(double, GetPointerDy, (libinput_event_pointer & event), (override));
    MOCK_METHOD(double, GetPointerAbsoluteX, (libinput_event_pointer & event, std::uint32_t width), (override));
    MOCK_METHOD(double, GetPointerAbsoluteY, (libinput_event_pointer & event, std::uint32_t width), (override));
    MOCK_METHOD(std::uint32_t, GetPointerButton, (libinput_event_pointer & event), (override));
    MOCK_METHOD(libinput_button_state, GetPointerButtonState, (libinput_event_pointer & event), (override));
    MOCK_METHOD(bool, GetPointerHasAxis, (libinput_event_pointer & event, libinput_pointer_axis axis), (override));
    MOCK_METHOD(
        double,
        GetPointerScrollValue,
        (libinput_event_pointer & event, libinput_pointer_axis axis),
        (override)
    );
    MOCK_METHOD(libinput_device*, GetDevice, (libinput_event & event), (override));
    MOCK_METHOD(int, GetDeviceFingerCount, (libinput_device & device), (override));
    MOCK_METHOD(
        bool,
        DeviceHasCapability,
        (libinput_device & device, libinput_device_capability capability),
        (override)
    );
    MOCK_METHOD(const char*, GetDeviceName, (libinput_device & device), (override));
    MOCK_METHOD(libinput_event_tablet_tool*, GetTabletToolEvent, (libinput_event & event), (override));
    MOCK_METHOD(std::uint64_t, GetTabletToolTimeMicroseconds, (libinput_event_tablet_tool & event), (override));
    MOCK_METHOD(double, GetTabletToolDx, (libinput_event_tablet_tool & event), (override));
    MOCK_METHOD(double, GetTabletToolDy, (libinput_event_tablet_tool & event), (override));
    MOCK_METHOD(std::uint32_t, GetTabletToolButton, (libinput_event_tablet_tool & event), (override));
    MOCK_METHOD(libinput_button_state, GetTabletToolButtonState, (libinput_event_tablet_tool & event), (override));
    MOCK_METHOD(
        libinput_tablet_tool_tip_state,
        GetTabletToolTipState,
        (libinput_event_tablet_tool & event),
        (override)
    );
    MOCK_METHOD(
        libinput_tablet_tool_proximity_state,
        GetTabletToolProximityState,
        (libinput_event_tablet_tool & event),
        (override)
    );
    MOCK_METHOD(libinput_event_tablet_pad*, GetTabletPadEvent, (libinput_event & event), (override));
    MOCK_METHOD(std::uint64_t, GetTabletPadTimeMicroseconds, (libinput_event_tablet_pad & event), (override));
    MOCK_METHOD(std::uint32_t, GetTabletPadButtonNumber, (libinput_event_tablet_pad & event), (override));
    MOCK_METHOD(libinput_button_state, GetTabletPadButtonState, (libinput_event_tablet_pad & event), (override));
    MOCK_METHOD(std::uint32_t, GetTabletPadKey, (libinput_event_tablet_pad & event), (override));
    MOCK_METHOD(libinput_key_state, GetTabletPadKeyState, (libinput_event_tablet_pad & event), (override));
    MOCK_METHOD(libinput_event_touch*, GetTouchEvent, (libinput_event & event), (override));
    MOCK_METHOD(std::uint64_t, GetTouchTimeMicroseconds, (libinput_event_touch & event), (override));
    MOCK_METHOD(double, GetTouchX, (libinput_event_touch & event, std::uint32_t width), (override));
    MOCK_METHOD(double, GetTouchY, (libinput_event_touch & event, std::uint32_t height), (override));
    MOCK_METHOD(std::int32_t, GetTouchSeatSlot, (libinput_event_touch & event), (override));
    MOCK_METHOD(libinput_event_keyboard*, GetKeyboardEvent, (libinput_event & event), (override));
    MOCK_METHOD(std::uint64_t, GetKeyboardTimeMicroseconds, (libinput_event_keyboard & event), (override));
    MOCK_METHOD(std::uint32_t, GetKeyboardKey, (libinput_event_keyboard & event), (override));
    MOCK_METHOD(libinput_key_state, GetKeyboardKeyState, (libinput_event_keyboard & event), (override));
};

} // namespace test

#endif // EVGETLIBINPUT_TEST_COMMON_TEST_HELPERS_H
