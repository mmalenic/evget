#include "common/test_helpers.h"

#include <utility>

void test::NoopDestroyLibInputEvent(libinput_event* /* unused */) noexcept {}

evgetlibinput::LibInputEvent test::WrapLibInputEvent(libinput_event* event) {
    return evgetlibinput::LibInputEvent{event, NoopDestroyLibInputEvent};
}

evgetlibinput::XkbCommon test::MakeUsXkb() {
    auto xkb = evgetlibinput::XkbCommon::NewWithNames(&kUsLayout);
    return std::move(xkb).value();
}

evget::InputEvent<evgetlibinput::LibInputEvent> test::MakeInputEvent() {
    return evget::InputEvent{WrapLibInputEvent(&g_event)};
}

void test::SetCommonMocks(LibInputApiMock& mock, libinput_event_type event_type) {
    EXPECT_CALL(mock, GetDevice(testing::_)).WillRepeatedly(testing::Return(&g_device));
    EXPECT_CALL(mock, GetDeviceName(testing::_)).WillRepeatedly(testing::Return(kDeviceName));
    EXPECT_CALL(mock, GetEventType(testing::_)).WillRepeatedly(testing::Return(event_type));
    EXPECT_CALL(mock, GetDeviceFingerCount(testing::_)).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(mock, DeviceHasCapability(testing::_, LIBINPUT_DEVICE_CAP_TOUCH))
        .WillRepeatedly(testing::Return(false));
    EXPECT_CALL(mock, DeviceHasCapability(testing::_, LIBINPUT_DEVICE_CAP_POINTER))
        .WillRepeatedly(testing::Return(true));
}
