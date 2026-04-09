// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
#include "evgetlibinput/event_transformer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <libinput.h>
#include <linux/input-event-codes.h>

#include <string>
#include <utility>

#include "common/test_helpers.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evget/input_event.h"
#include "evgetlibinput/libinput.h"

// libinput exposes its event/device handles only as forward-declared opaque structs. Real types must
// be defined here for the code to compile. The completions are local to this TU and never participate
// in linkage with libinput's own definitions because the mocks never call into libinput.
struct libinput_event {};

struct libinput_device {};

struct libinput_event_pointer {};

struct libinput_event_keyboard {};

struct libinput_event_touch {};

struct libinput_event_tablet_tool {};

struct libinput_event_tablet_pad {};

namespace {

using test::kDeviceName;
using test::kDimensions;
using test::MakeUsXkb;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

void NoopDestroyLibInputEvent(libinput_event* /*event*/) noexcept {} // NOLINT(readability/casting)

evgetlibinput::LibInputEvent WrapLibInputEvent(libinput_event* event) {
    return evgetlibinput::LibInputEvent{event, NoopDestroyLibInputEvent};
}

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
libinput_event g_event{};
libinput_device g_device{};
libinput_event_pointer g_pointer_event{};
libinput_event_keyboard g_keyboard_event{};
libinput_event_touch g_touch_event{};
libinput_event_tablet_tool g_tablet_tool_event{};
libinput_event_tablet_pad g_tablet_pad_event{};

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

evget::InputEvent<evgetlibinput::LibInputEvent> MakeInputEvent() {
    return evget::InputEvent{WrapLibInputEvent(&g_event)};
}

void SetCommonMocks(test::LibInputApiMock& mock, libinput_event_type event_type) {
    EXPECT_CALL(mock, GetDevice(_)).WillRepeatedly(Return(&g_device));
    EXPECT_CALL(mock, GetDeviceName(_)).WillRepeatedly(Return(kDeviceName));
    EXPECT_CALL(mock, GetEventType(_)).WillRepeatedly(Return(event_type));
    EXPECT_CALL(mock, GetDeviceFingerCount(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(mock, DeviceHasCapability(_, LIBINPUT_DEVICE_CAP_TOUCH)).WillRepeatedly(Return(false));
    EXPECT_CALL(mock, DeviceHasCapability(_, LIBINPUT_DEVICE_CAP_POINTER)).WillRepeatedly(Return(true));
}

} // namespace

TEST(EvgetLibInputTransformer, TransformNullEventEmpty) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};

    evget::InputEvent input_event{evgetlibinput::LibInputEvent{nullptr, NoopDestroyLibInputEvent}};

    auto data = transformer.TransformEvent(std::move(input_event));
    ASSERT_TRUE(data.Empty());
}

TEST(EvgetLibInputTransformer, TransformNullDeviceEmpty) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    EXPECT_CALL(libinput_mock, GetDevice(_)).WillOnce(Return(nullptr));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());

    ASSERT_TRUE(data.Empty());
}

TEST(EvgetLibInputTransformer, TransformPointerMotion) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_MOTION);
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(1000));
    EXPECT_CALL(libinput_mock, GetPointerDx(_)).WillOnce(Return(2.5));
    EXPECT_CALL(libinput_mock, GetPointerDy(_)).WillOnce(Return(-1.5));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(0), "");
    ASSERT_FALSE(entries.at(0).Data().at(1).empty());
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(2.5));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(-1.5));
    ASSERT_EQ(entries.at(0).Data().at(4), kDeviceName);
    ASSERT_FALSE(entries.at(0).Data().at(11).empty());
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_POINTER_MOTION");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
}

TEST(EvgetLibInputTransformer, TransformPointerMotionSecondInterval) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_MOTION);
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillOnce(Return(1000)).WillOnce(Return(3000));
    EXPECT_CALL(libinput_mock, GetPointerDx(_)).WillRepeatedly(Return(0.0));
    EXPECT_CALL(libinput_mock, GetPointerDy(_)).WillRepeatedly(Return(0.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto first = transformer.TransformEvent(MakeInputEvent());
    auto second = transformer.TransformEvent(MakeInputEvent());

    ASSERT_EQ(first.Entries().at(0).Data().at(0), "");
    ASSERT_EQ(second.Entries().at(0).Data().at(0), "2000");
}

TEST(EvgetLibInputTransformer, TransformPointerAbsoluteMotionFirstNoDelta) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(2000));
    EXPECT_CALL(libinput_mock, GetPointerAbsoluteX(_, kDimensions.width)).WillOnce(Return(100.0));
    EXPECT_CALL(libinput_mock, GetPointerAbsoluteY(_, kDimensions.height)).WillOnce(Return(200.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    // No previous absolute position, so no PositionX/Y is set.
    ASSERT_EQ(entries.at(0).Data().at(2), "");
    ASSERT_EQ(entries.at(0).Data().at(3), "");
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE");
}

TEST(EvgetLibInputTransformer, TransformPointerAbsoluteMotionSecondComputesDelta) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE);
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(2000));
    EXPECT_CALL(libinput_mock, GetPointerAbsoluteX(_, kDimensions.width))
        .WillOnce(Return(100.0))
        .WillOnce(Return(110.0));
    EXPECT_CALL(libinput_mock, GetPointerAbsoluteY(_, kDimensions.height))
        .WillOnce(Return(200.0))
        .WillOnce(Return(195.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    static_cast<void>(transformer.TransformEvent(MakeInputEvent()));
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(10.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(-5.0));
}

TEST(EvgetLibInputTransformer, TransformPointerButton) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_BUTTON);
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(3000));
    EXPECT_CALL(libinput_mock, GetPointerButton(_)).WillOnce(Return(BTN_LEFT));
    EXPECT_CALL(libinput_mock, GetPointerButtonState(_)).WillOnce(Return(LIBINPUT_BUTTON_STATE_PRESSED));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_POINTER_BUTTON");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
    ASSERT_EQ(entries.at(0).Data().at(16), std::to_string(BTN_LEFT));
    ASSERT_EQ(entries.at(0).Data().at(17), "BTN_LEFT");
    ASSERT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(EvgetLibInputTransformer, TransformPointerButtonRelease) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_BUTTON);
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(3000));
    EXPECT_CALL(libinput_mock, GetPointerButton(_)).WillOnce(Return(BTN_RIGHT));
    EXPECT_CALL(libinput_mock, GetPointerButtonState(_)).WillOnce(Return(LIBINPUT_BUTTON_STATE_RELEASED));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(16), std::to_string(BTN_RIGHT));
    ASSERT_EQ(entries.at(0).Data().at(17), "BTN_RIGHT");
    ASSERT_EQ(entries.at(0).Data().at(18), "1");
}

TEST(EvgetLibInputTransformer, TransformPointerScrollWheel) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_SCROLL_WHEEL);
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(4000));
    EXPECT_CALL(libinput_mock, GetPointerHasAxis(_, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)).WillOnce(Return(true));
    EXPECT_CALL(libinput_mock, GetPointerHasAxis(_, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)).WillOnce(Return(false));
    EXPECT_CALL(libinput_mock, GetPointerScrollValue(_, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)).WillOnce(Return(15.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_POINTER_SCROLL_WHEEL");
    ASSERT_EQ(entries.at(0).Data().at(15), evget::FromDouble(15.0));
    ASSERT_EQ(entries.at(0).Data().at(16), "");
}

TEST(EvgetLibInputTransformer, TransformKeyboardKeyNoModifiers) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_KEYBOARD_KEY);
    EXPECT_CALL(libinput_mock, GetKeyboardEvent(_)).WillRepeatedly(Return(&g_keyboard_event));
    EXPECT_CALL(libinput_mock, GetKeyboardTimeMicroseconds(_)).WillRepeatedly(Return(5000));
    EXPECT_CALL(libinput_mock, GetKeyboardKey(_)).WillOnce(Return(KEY_A));
    EXPECT_CALL(libinput_mock, GetKeyboardKeyState(_)).WillRepeatedly(Return(LIBINPUT_KEY_STATE_PRESSED));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_KEYBOARD_KEY");
    ASSERT_EQ(entries.at(0).Data().at(14), "1");
    ASSERT_EQ(entries.at(0).Data().at(15), std::to_string(KEY_A));
    ASSERT_EQ(entries.at(0).Data().at(16), "a");
    ASSERT_EQ(entries.at(0).Data().at(17), "a");
    ASSERT_EQ(entries.at(0).Data().at(18), "0");
    ASSERT_EQ(entries.at(0).Modifiers().size(), 0);
}

TEST(EvgetLibInputTransformer, TransformKeyboardKeyShiftHeld) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_KEYBOARD_KEY);
    EXPECT_CALL(libinput_mock, GetKeyboardEvent(_)).WillRepeatedly(Return(&g_keyboard_event));
    EXPECT_CALL(libinput_mock, GetKeyboardTimeMicroseconds(_)).WillRepeatedly(Return(5000));
    // Sequence: press shift, press A (shifted), release shift, press A (unshifted).
    EXPECT_CALL(libinput_mock, GetKeyboardKey(_))
        .WillOnce(Return(KEY_LEFTSHIFT))
        .WillOnce(Return(KEY_A))
        .WillOnce(Return(KEY_LEFTSHIFT))
        .WillOnce(Return(KEY_A));
    EXPECT_CALL(libinput_mock, GetKeyboardKeyState(_))
        .WillOnce(Return(LIBINPUT_KEY_STATE_PRESSED))
        .WillOnce(Return(LIBINPUT_KEY_STATE_PRESSED))
        .WillOnce(Return(LIBINPUT_KEY_STATE_RELEASED))
        .WillOnce(Return(LIBINPUT_KEY_STATE_PRESSED));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    // Press shift first so xkb's modifier state reflects it for the next event.
    transformer.TransformEvent(MakeInputEvent());
    auto shifted = transformer.TransformEvent(MakeInputEvent());
    // Release shift; xkb's modifier state should clear before the next KEY_A.
    transformer.TransformEvent(MakeInputEvent());
    auto unshifted = transformer.TransformEvent(MakeInputEvent());

    const auto& shifted_entries = shifted.Entries();
    ASSERT_EQ(shifted_entries.size(), 1);
    ASSERT_EQ(shifted_entries.at(0).Type(), evget::EntryType::kKey);
    ASSERT_EQ(shifted_entries.at(0).Data().at(12), "LIBINPUT_EVENT_KEYBOARD_KEY");
    ASSERT_EQ(shifted_entries.at(0).Data().at(14), "1");
    ASSERT_EQ(shifted_entries.at(0).Data().at(15), std::to_string(KEY_A));
    ASSERT_EQ(shifted_entries.at(0).Data().at(16), "A");
    ASSERT_EQ(shifted_entries.at(0).Data().at(17), "A");
    ASSERT_EQ(shifted_entries.at(0).Data().at(18), "0");
    ASSERT_EQ(shifted_entries.at(0).Modifiers().size(), 1);
    ASSERT_EQ(shifted_entries.at(0).Modifiers().at(0), "0");

    const auto& unshifted_entries = unshifted.Entries();
    ASSERT_EQ(unshifted_entries.size(), 1);
    ASSERT_EQ(unshifted_entries.at(0).Type(), evget::EntryType::kKey);
    ASSERT_EQ(unshifted_entries.at(0).Data().at(15), std::to_string(KEY_A));
    ASSERT_EQ(unshifted_entries.at(0).Data().at(16), "a");
    ASSERT_EQ(unshifted_entries.at(0).Data().at(17), "a");
    ASSERT_EQ(unshifted_entries.at(0).Data().at(18), "0");
    ASSERT_EQ(unshifted_entries.at(0).Modifiers().size(), 0);
}

TEST(EvgetLibInputTransformer, TransformTouchDownProducesMoveAndPress) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TOUCH_DOWN);
    // Touchscreen capability so the device type is reported as kTouchscreen.
    EXPECT_CALL(libinput_mock, DeviceHasCapability(_, LIBINPUT_DEVICE_CAP_TOUCH)).WillRepeatedly(Return(true));
    EXPECT_CALL(libinput_mock, GetTouchEvent(_)).WillRepeatedly(Return(&g_touch_event));
    EXPECT_CALL(libinput_mock, GetTouchTimeMicroseconds(_)).WillRepeatedly(Return(6000));
    EXPECT_CALL(libinput_mock, GetTouchSeatSlot(_)).WillRepeatedly(Return(7));
    EXPECT_CALL(libinput_mock, GetTouchX(_, kDimensions.width)).WillOnce(Return(50.0));
    EXPECT_CALL(libinput_mock, GetTouchY(_, kDimensions.height)).WillOnce(Return(60.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_TOUCH_DOWN");
    ASSERT_EQ(entries.at(0).Data().at(14), "3");
    ASSERT_EQ(entries.at(0).Data().at(15), "7");
    ASSERT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(1).Data().at(15), "7");
    ASSERT_EQ(entries.at(1).Data().at(18), "0");
}

TEST(EvgetLibInputTransformer, TransformTouchMotionComputesDelta) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TOUCH_DOWN);
    EXPECT_CALL(libinput_mock, DeviceHasCapability(_, LIBINPUT_DEVICE_CAP_TOUCH)).WillRepeatedly(Return(true));
    EXPECT_CALL(libinput_mock, GetTouchEvent(_)).WillRepeatedly(Return(&g_touch_event));
    EXPECT_CALL(libinput_mock, GetTouchTimeMicroseconds(_)).WillRepeatedly(Return(6000));
    EXPECT_CALL(libinput_mock, GetTouchSeatSlot(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(libinput_mock, GetTouchX(_, kDimensions.width)).WillOnce(Return(10.0)).WillOnce(Return(13.0));
    EXPECT_CALL(libinput_mock, GetTouchY(_, kDimensions.height)).WillOnce(Return(20.0)).WillOnce(Return(24.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    // The first TOUCH_DOWN seeds the previous touch position.
    static_cast<void>(transformer.TransformEvent(MakeInputEvent()));

    EXPECT_CALL(libinput_mock, GetEventType(_)).WillRepeatedly(Return(LIBINPUT_EVENT_TOUCH_MOTION));
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(3.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(4.0));
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_TOUCH_MOTION");
}

TEST(EvgetLibInputTransformer, TransformTouchUpProducesMoveAndRelease) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TOUCH_UP);
    EXPECT_CALL(libinput_mock, DeviceHasCapability(_, LIBINPUT_DEVICE_CAP_TOUCH)).WillRepeatedly(Return(true));
    EXPECT_CALL(libinput_mock, GetTouchEvent(_)).WillRepeatedly(Return(&g_touch_event));
    EXPECT_CALL(libinput_mock, GetTouchTimeMicroseconds(_)).WillRepeatedly(Return(7000));
    EXPECT_CALL(libinput_mock, GetTouchSeatSlot(_)).WillRepeatedly(Return(2));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(2), "");
    ASSERT_EQ(entries.at(0).Data().at(15), "2");
    ASSERT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(1).Data().at(12), "LIBINPUT_EVENT_TOUCH_UP");
    ASSERT_EQ(entries.at(1).Data().at(15), "2");
    ASSERT_EQ(entries.at(1).Data().at(18), "1");
}

TEST(EvgetLibInputTransformer, TransformTabletToolAxis) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TABLET_TOOL_AXIS);
    EXPECT_CALL(libinput_mock, GetTabletToolEvent(_)).WillRepeatedly(Return(&g_tablet_tool_event));
    EXPECT_CALL(libinput_mock, GetTabletToolTimeMicroseconds(_)).WillRepeatedly(Return(8000));
    EXPECT_CALL(libinput_mock, GetTabletToolDx(_)).WillOnce(Return(1.25));
    EXPECT_CALL(libinput_mock, GetTabletToolDy(_)).WillOnce(Return(-0.75));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.25));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(-0.75));
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_TABLET_TOOL_AXIS");
    ASSERT_EQ(entries.at(0).Data().at(14), "4");
}

TEST(EvgetLibInputTransformer, TransformTabletToolButton) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TABLET_TOOL_BUTTON);
    EXPECT_CALL(libinput_mock, GetTabletToolEvent(_)).WillRepeatedly(Return(&g_tablet_tool_event));
    EXPECT_CALL(libinput_mock, GetTabletToolTimeMicroseconds(_)).WillRepeatedly(Return(9000));
    EXPECT_CALL(libinput_mock, GetTabletToolButton(_)).WillOnce(Return(BTN_STYLUS));
    EXPECT_CALL(libinput_mock, GetTabletToolButtonState(_)).WillOnce(Return(LIBINPUT_BUTTON_STATE_PRESSED));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_TABLET_TOOL_BUTTON");
    ASSERT_EQ(entries.at(0).Data().at(14), "4");
    ASSERT_EQ(entries.at(0).Data().at(16), std::to_string(BTN_STYLUS));
    ASSERT_EQ(entries.at(0).Data().at(17), "BTN_STYLUS");
    ASSERT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(EvgetLibInputTransformer, TransformTabletPadButton) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TABLET_PAD_BUTTON);
    EXPECT_CALL(libinput_mock, GetTabletPadEvent(_)).WillRepeatedly(Return(&g_tablet_pad_event));
    EXPECT_CALL(libinput_mock, GetTabletPadTimeMicroseconds(_)).WillRepeatedly(Return(10000));
    EXPECT_CALL(libinput_mock, GetTabletPadButtonNumber(_)).WillOnce(Return(3));
    EXPECT_CALL(libinput_mock, GetTabletPadButtonState(_)).WillOnce(Return(LIBINPUT_BUTTON_STATE_RELEASED));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_TABLET_PAD_BUTTON");
    ASSERT_EQ(entries.at(0).Data().at(14), "4");
    ASSERT_EQ(entries.at(0).Data().at(16), "3");
    ASSERT_EQ(entries.at(0).Data().at(18), "1");
}

TEST(EvgetLibInputTransformer, TransformTabletPadKey) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TABLET_PAD_KEY);
    EXPECT_CALL(libinput_mock, GetTabletPadEvent(_)).WillRepeatedly(Return(&g_tablet_pad_event));
    EXPECT_CALL(libinput_mock, GetTabletPadTimeMicroseconds(_)).WillRepeatedly(Return(11000));
    EXPECT_CALL(libinput_mock, GetTabletPadKey(_)).WillOnce(Return(KEY_F1));
    EXPECT_CALL(libinput_mock, GetTabletPadKeyState(_)).WillOnce(Return(LIBINPUT_KEY_STATE_PRESSED));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_TABLET_PAD_KEY");
    ASSERT_EQ(entries.at(0).Data().at(14), "1");
    ASSERT_EQ(entries.at(0).Data().at(15), std::to_string(KEY_F1));
    ASSERT_EQ(entries.at(0).Data().at(16), "KEY_F1");
    ASSERT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(EvgetLibInputTransformer, TransformTabletToolProximityInGeneratesMove) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
    EXPECT_CALL(libinput_mock, GetTabletToolEvent(_)).WillRepeatedly(Return(&g_tablet_tool_event));
    EXPECT_CALL(libinput_mock, GetTabletToolProximityState(_))
        .WillOnce(Return(LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN));
    EXPECT_CALL(libinput_mock, GetTabletToolTimeMicroseconds(_)).WillOnce(Return(12000));
    EXPECT_CALL(libinput_mock, GetTabletToolDx(_)).WillOnce(Return(0.0));
    EXPECT_CALL(libinput_mock, GetTabletToolDy(_)).WillOnce(Return(0.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(12), "LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY");
}

TEST(EvgetLibInputTransformer, TransformTabletToolProximityOutProducesNothing) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY);
    EXPECT_CALL(libinput_mock, GetTabletToolEvent(_)).WillRepeatedly(Return(&g_tablet_tool_event));
    EXPECT_CALL(libinput_mock, GetTabletToolProximityState(_))
        .WillOnce(Return(LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());

    ASSERT_TRUE(data.Empty());
}

TEST(EvgetLibInputTransformer, TransformTabletToolTipProducesMoveAndClick) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_TABLET_TOOL_TIP);
    EXPECT_CALL(libinput_mock, GetTabletToolEvent(_)).WillRepeatedly(Return(&g_tablet_tool_event));
    EXPECT_CALL(libinput_mock, GetTabletToolTimeMicroseconds(_)).WillRepeatedly(Return(13000));
    EXPECT_CALL(libinput_mock, GetTabletToolDx(_)).WillOnce(Return(2.0));
    EXPECT_CALL(libinput_mock, GetTabletToolDy(_)).WillOnce(Return(3.0));
    EXPECT_CALL(libinput_mock, GetTabletToolTipState(_)).WillOnce(Return(LIBINPUT_TABLET_TOOL_TIP_DOWN));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(2.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(3.0));
    ASSERT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(1).Data().at(18), "0");
}

TEST(EvgetLibInputTransformer, DeviceTypeTouchpadFromFingerCount) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_MOTION);
    EXPECT_CALL(libinput_mock, GetDeviceFingerCount(_)).WillRepeatedly(Return(2));
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(1000));
    EXPECT_CALL(libinput_mock, GetPointerDx(_)).WillOnce(Return(0.0));
    EXPECT_CALL(libinput_mock, GetPointerDy(_)).WillOnce(Return(0.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Data().at(14), "2");
}

TEST(EvgetLibInputTransformer, DeviceTypeUnknownWhenNoCapabilities) {
    NiceMock<test::LibInputApiMock> libinput_mock{};
    auto xkb = MakeUsXkb();

    SetCommonMocks(libinput_mock, LIBINPUT_EVENT_POINTER_MOTION);
    EXPECT_CALL(libinput_mock, DeviceHasCapability(_, LIBINPUT_DEVICE_CAP_POINTER)).WillRepeatedly(Return(false));
    EXPECT_CALL(libinput_mock, GetPointerEvent(_)).WillRepeatedly(Return(&g_pointer_event));
    EXPECT_CALL(libinput_mock, GetPointerTimeMicroseconds(_)).WillRepeatedly(Return(1000));
    EXPECT_CALL(libinput_mock, GetPointerDx(_)).WillOnce(Return(0.0));
    EXPECT_CALL(libinput_mock, GetPointerDy(_)).WillOnce(Return(0.0));

    evgetlibinput::EventTransformer transformer{libinput_mock, xkb, kDimensions};
    auto data = transformer.TransformEvent(MakeInputEvent());
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Data().at(14), "5");
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
