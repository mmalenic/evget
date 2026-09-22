// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
#include "evgetwindows/event_transformer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <windows.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "common/windows_mock.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evget/input_event.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/hid_query_api.h"
#include "evgetwindows/message_window.h"
#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows_query_api.h"

using test::AsRawInput;
using test::ExpectTouchParitySequence;
using test::HidDeviceHandle;
using test::HidDeviceHandleAlternate;
using test::HidDeviceHandleAt;
using test::HidQueryApiMock;
using test::kTestAxisMax;
using test::kTestMappedDisplay;
using test::kTestMonitorHeight;
using test::kTestMonitorWidth;
using test::kTestPointerDisplay;
using test::MakeAxisRange;
using test::MakeContact;
using test::MakeContactlessReport;
using test::MakeContactState;
using test::MakeDeviceChangeRawEvent;
using test::MakeHidFrame;
using test::MakeHidPacket;
using test::MakeHidPacketNullDevice;
using test::MakeHidPacketUndersized;
using test::MakeHidRawEvent;
using test::MakeHidRawEventFrom;
using test::MakeHidReport;
using test::MakeHidReportBytes;
using test::MakeHidReportFrom;
using test::MakeInjected;
using test::MakeKeyboard;
using test::MakeMappedMonitor;
using test::MakeMappedMonitorExtent;
using test::MakeMouseAbsolute;
using test::MakeMouseButton;
using test::MakeMouseMoveRelative;
using test::MakeMouseWheel;
using test::WindowsQueryApiMock;
using ::testing::NiceMock;
using ::testing::Return;

TEST(EvgetWindowsTransformer, MouseMoveRelativeHasChange) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseMoveRelative(5, -3)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(5.0));
    EXPECT_EQ(entries.at(0).Data().at(3), evget::FromDouble(-3.0));
    EXPECT_EQ(entries.at(0).Data().at(4), "windows-injected");
    EXPECT_FALSE(entries.at(0).Data().at(11).empty());
    EXPECT_EQ(entries.at(0).Data().at(12), "MOUSE_MOVE_RELATIVE");
    EXPECT_EQ(entries.at(0).Data().at(13), "windows");
    EXPECT_EQ(entries.at(0).Data().at(14), "0");
}

TEST(EvgetWindowsTransformer, MouseMoveAbsoluteFirstHasNoPosition) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseAbsolute(100, 200)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), "");
    EXPECT_EQ(entries.at(0).Data().at(3), "");
    EXPECT_EQ(entries.at(0).Data().at(12), "MOUSE_MOVE_ABSOLUTE");
}

TEST(EvgetWindowsTransformer, MouseMoveAbsoluteSecondHasChange) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    static_cast<void>(
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseAbsolute(100, 200)})
    );
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseAbsolute(110, 195)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(10.0));
    EXPECT_EQ(entries.at(0).Data().at(3), evget::FromDouble(-5.0));
}

TEST(EvgetWindowsTransformer, ScrollWheelUpNegatesSign) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseWheel(WHEEL_DELTA, false)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    EXPECT_EQ(entries.at(0).Data().at(12), "RI_MOUSE_WHEEL");
    EXPECT_EQ(entries.at(0).Data().at(15), evget::FromDouble(-1.0));
    EXPECT_EQ(entries.at(0).Data().at(16), "");
}

TEST(EvgetWindowsTransformer, ScrollWheelDownNegatesSign) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseWheel(-WHEEL_DELTA, false)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    EXPECT_EQ(entries.at(0).Data().at(15), evget::FromDouble(1.0));
    EXPECT_EQ(entries.at(0).Data().at(16), "");
}

TEST(EvgetWindowsTransformer, ScrollHorizontalHasSign) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseWheel(WHEEL_DELTA, true)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    EXPECT_EQ(entries.at(0).Data().at(12), "RI_MOUSE_HWHEEL");
    EXPECT_EQ(entries.at(0).Data().at(15), "");
    EXPECT_EQ(entries.at(0).Data().at(16), evget::FromDouble(1.0));
}

TEST(EvgetWindowsTransformer, ScrollHorizontalNegativeHasSign) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseWheel(-WHEEL_DELTA, true)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    EXPECT_EQ(entries.at(0).Data().at(16), evget::FromDouble(-1.0));
}

TEST(EvgetWindowsTransformer, MouseClickLeftPressUsesEvdevCode) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeMouseButton(RI_MOUSE_LEFT_BUTTON_DOWN)}
    );
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(0).Data().at(12), "RI_MOUSE_LEFT_BUTTON_DOWN");
    EXPECT_EQ(entries.at(0).Data().at(16), std::to_string(0x110));
    EXPECT_EQ(entries.at(0).Data().at(17), "BTN_LEFT");
    EXPECT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(EvgetWindowsTransformer, MouseClickRightReleaseUsesEvdevCode) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeMouseButton(RI_MOUSE_RIGHT_BUTTON_UP)}
    );
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(0).Data().at(12), "RI_MOUSE_RIGHT_BUTTON_UP");
    EXPECT_EQ(entries.at(0).Data().at(16), std::to_string(0x111));
    EXPECT_EQ(entries.at(0).Data().at(17), "BTN_RIGHT");
    EXPECT_EQ(entries.at(0).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, MouseClickCoversAllButtons) {
    struct ButtonCase {
        USHORT flag;
        int expected_id;
        std::string expected_name;
    };

    const std::array<ButtonCase, 5> cases{{
        {.flag = RI_MOUSE_LEFT_BUTTON_DOWN, .expected_id = 0x110, .expected_name = "BTN_LEFT"},
        {.flag = RI_MOUSE_RIGHT_BUTTON_DOWN, .expected_id = 0x111, .expected_name = "BTN_RIGHT"},
        {.flag = RI_MOUSE_MIDDLE_BUTTON_DOWN, .expected_id = 0x112, .expected_name = "BTN_MIDDLE"},
        {.flag = RI_MOUSE_BUTTON_4_DOWN, .expected_id = 0x113, .expected_name = "BTN_SIDE"},
        {.flag = RI_MOUSE_BUTTON_5_DOWN, .expected_id = 0x114, .expected_name = "BTN_EXTRA"},
    }};

    for (const auto& button : cases) {
        NiceMock<WindowsQueryApiMock> query{};
        NiceMock<HidQueryApiMock> hid_query{};
        evgetwindows::ModifierTracker tracker{};
        evgetwindows::EventTransformer transformer{query, hid_query, tracker};

        auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseButton(button.flag)});
        const auto& entries = data.Entries();

        ASSERT_EQ(entries.size(), 1) << "button id " << button.expected_id;
        EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
        const auto& fields = entries.at(0).Data();
        EXPECT_EQ(
            (std::vector{fields.at(16), fields.at(17), fields.at(18)}),
            (std::vector{std::to_string(button.expected_id), button.expected_name, std::string{"0"}})
        );
    }
}

TEST(EvgetWindowsTransformer, CharacterFieldIsApplied) {
    NiceMock<WindowsQueryApiMock> query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, CharacterFor(static_cast<UINT>('A'), testing::_, testing::_))
        .WillOnce(Return(std::optional<std::string>{"a"}));

    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeKeyboard('A', 0x1E, 0)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    EXPECT_EQ(entries.at(0).Data().at(14), "1");
    EXPECT_EQ(entries.at(0).Data().at(15), std::to_string(static_cast<int>('A')));
    EXPECT_EQ(entries.at(0).Data().at(16), "a");
    EXPECT_EQ(entries.at(0).Data().at(17), "a");
    EXPECT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(EvgetWindowsTransformer, DeadKeyHasCharacterUnset) {
    NiceMock<WindowsQueryApiMock> query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, CharacterFor(testing::_, testing::_, testing::_)).WillRepeatedly(Return(std::nullopt));

    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeKeyboard('A', 0x1E, 0)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    EXPECT_EQ(entries.at(0).Data().at(16), "a");
    EXPECT_EQ(entries.at(0).Data().at(17), "");
}

TEST(EvgetWindowsTransformer, KeyReleaseHasNoCharacterLookup) {
    NiceMock<WindowsQueryApiMock> query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, CharacterFor(testing::_, testing::_, testing::_)).Times(0);

    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    auto data =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeKeyboard('A', 0x1E, RI_KEY_BREAK)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    EXPECT_EQ(entries.at(0).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, ModifiersHaveValues) {
    constexpr USHORT kLeftShiftScan = 0x2A;
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto shift_press = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeKeyboard(VK_LSHIFT, kLeftShiftScan, 0)}
    );
    auto next_key = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeKeyboard('A', 0x1E, 0)});

    ASSERT_EQ(shift_press.Entries().size(), 1);
    EXPECT_EQ(shift_press.Entries().at(0).Modifiers().size(), 0);

    ASSERT_EQ(next_key.Entries().size(), 1);
    EXPECT_EQ(next_key.Entries().at(0).Modifiers().size(), 1);
}

TEST(EvgetWindowsTransformer, DeviceNameResolved) {
    int device_backing = 0;
    HANDLE handle = &device_backing;

    NiceMock<WindowsQueryApiMock> query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, DeviceName(handle)).WillRepeatedly(Return(std::optional<std::string>{"Test Mouse"}));

    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto raw = MakeMouseMoveRelative(1, 1);
    raw.header.hDevice = handle;
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{raw});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Data().at(4), "Test Mouse");
}

TEST(EvgetWindowsTransformer, InjectedDeviceSkipsLookup) {
    NiceMock<WindowsQueryApiMock> query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, DeviceName(testing::_)).Times(0);

    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeInjected(VK_RETURN)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Data().at(4), "windows-injected");
}

TEST(EvgetWindowsTransformer, FocusWindowPresentHasFields) {
    NiceMock<WindowsQueryApiMock> query{};
    evgetwindows::ModifierTracker tracker{};
    const evgetwindows::FocusWindowInfo info{
        .name = "Editor",
        .position_x = 10.0,
        .position_y = 20.0,
        .width = 800.0,
        .height = 600.0,
    };
    EXPECT_CALL(query, FocusWindow()).WillRepeatedly(Return(std::optional{info}));

    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseMoveRelative(1, 1)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Data().at(5), "Editor");
    EXPECT_EQ(entries.at(0).Data().at(6), evget::FromDouble(10.0));
    EXPECT_EQ(entries.at(0).Data().at(7), evget::FromDouble(20.0));
    EXPECT_EQ(entries.at(0).Data().at(8), evget::FromDouble(800.0));
    EXPECT_EQ(entries.at(0).Data().at(9), evget::FromDouble(600.0));
    EXPECT_EQ(entries.at(0).Data().at(10), "1");
}

TEST(EvgetWindowsTransformer, FocusWindowAbsentHasFieldsEmpty) {
    NiceMock<WindowsQueryApiMock> query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, FocusWindow()).WillRepeatedly(Return(std::nullopt));

    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseMoveRelative(1, 1)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Data().at(5), "");
    EXPECT_EQ(entries.at(0).Data().at(6), "");
    EXPECT_EQ(entries.at(0).Data().at(10), "");
}

TEST(EvgetWindowsTransformer, HidEventNoEntries) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    evgetwindows::RawEvent raw{};
    raw.header.dwType = RIM_TYPEHID;
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{raw});

    EXPECT_TRUE(data.Empty());
}

TEST(EvgetWindowsTransformer, SchemaFieldsMatch) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto move = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseMoveRelative(1, 1)});
    ASSERT_EQ(move.Entries().size(), 1);
    EXPECT_EQ(move.Entries().at(0).Data().size(), evget::detail::kMouseMoveNFields);
    EXPECT_EQ(move.Entries().at(0).Data().at(13), "windows");

    auto scroll =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseWheel(WHEEL_DELTA, false)});
    ASSERT_EQ(scroll.Entries().size(), 1);
    EXPECT_EQ(scroll.Entries().at(0).Data().size(), evget::detail::kMouseScrollNFields);
    EXPECT_EQ(scroll.Entries().at(0).Data().at(13), "windows");

    auto click = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeMouseButton(RI_MOUSE_LEFT_BUTTON_DOWN)}
    );
    ASSERT_EQ(click.Entries().size(), 1);
    EXPECT_EQ(click.Entries().at(0).Data().size(), evget::detail::kMouseClickNFields);
    EXPECT_EQ(click.Entries().at(0).Data().at(13), "windows");

    auto key = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeKeyboard('A', 0x1E, 0)});
    ASSERT_EQ(key.Entries().size(), 1);
    EXPECT_EQ(key.Entries().at(0).Data().size(), evget::detail::kKeyNFields);
    EXPECT_EQ(key.Entries().at(0).Data().at(13), "windows");
}

TEST(EvgetWindowsTransformer, DownCreatesMoveAndPress) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidReport(7, true, true)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacket(report, 1);
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet), packet.size());
    ASSERT_TRUE(raw_event.has_value());

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{*raw_event});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);

    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), "");
    EXPECT_EQ(entries.at(0).Data().at(3), "");
    EXPECT_EQ(entries.at(0).Data().at(12), "RIM_TYPEHID");
    EXPECT_EQ(entries.at(0).Data().at(13), "windows");
    EXPECT_EQ(entries.at(0).Data().at(14), "3");
    EXPECT_EQ(entries.at(0).Data().at(15), "7");

    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(12), "RIM_TYPEHID");
    EXPECT_EQ(entries.at(1).Data().at(14), "3");
    EXPECT_EQ(entries.at(1).Data().at(15), "7");
    EXPECT_EQ(entries.at(1).Data().at(16), "");
    EXPECT_EQ(entries.at(1).Data().at(17), "");
    EXPECT_EQ(entries.at(1).Data().at(18), "0");
}

TEST(EvgetWindowsTransformer, DownUsesTouchpadDeviceType) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchpad));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidReport(7, true, true)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacket(report, 1);
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet), packet.size());
    ASSERT_TRUE(raw_event.has_value());

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{*raw_event});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).Data().at(14), "2");
    EXPECT_EQ(entries.at(1).Data().at(14), "2");
}

TEST(EvgetWindowsTransformer, TouchIgnoreUnknownHid) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kUnknown));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_)).Times(0);

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacket(report, 1);
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet), packet.size());
    ASSERT_TRUE(raw_event.has_value());

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{*raw_event});

    EXPECT_EQ(data.Entries().size(), 0);
}

TEST(EvgetWindowsTransformer, TouchIgnoreNull) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_)).Times(0);

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacketNullDevice(report, 1);
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet), packet.size());
    ASSERT_TRUE(raw_event.has_value());

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{*raw_event});

    EXPECT_EQ(data.Entries().size(), 0);
}

TEST(EvgetWindowsTransformer, TouchInvalidReport) {
    const auto report = MakeHidReportBytes(evgetwindows::kHidReportCapacity + 1);
    const auto packet = MakeHidPacket(report, 1);

    EXPECT_FALSE(evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet), packet.size()).has_value());
}

TEST(EvgetWindowsTransformer, TouchInconsistentPackets) {
    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacketUndersized(report, 1);

    EXPECT_FALSE(evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet), packet.size()).has_value());
}

TEST(EvgetWindowsTransformer, DeviceRemovalRemovesHid) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};

    int backing = 0;
    HANDLE device = &backing;
    EXPECT_CALL(hid_query, RemoveDevice(device)).Times(1);

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeDeviceChangeRawEvent(device, false)});

    EXPECT_TRUE(data.Entries().empty());
}

TEST(EvgetWindowsTransformer, DeviceArrivingHasNoRows) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};

    int backing = 0;
    HANDLE device = &backing;
    EXPECT_CALL(hid_query, RemoveDevice(testing::_)).Times(0);

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};

    auto data =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeDeviceChangeRawEvent(device, true)});

    EXPECT_TRUE(data.Entries().empty());
}

TEST(EvgetWindowsTransformer, TouchMotionhasPixelChanges) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidReportFrom({MakeContact(1, 0, 0)})}))
        .WillOnce(Return(std::optional{MakeHidReportFrom({MakeContact(1, kTestAxisMax / 16, 0)})}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(kTestMonitorWidth / 16));
    EXPECT_EQ(entries.at(0).Data().at(3), evget::FromDouble(0.0));
}

TEST(EvgetWindowsTransformer, TouchMotionParallelContacts) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidReportFrom({MakeContact(2, 0, 0), MakeContact(1, 0, 0)})}))
        .WillOnce(Return(
            std::optional{
                MakeHidReportFrom({MakeContact(2, kTestAxisMax / 8, 0), MakeContact(1, kTestAxisMax / 16, 0)})
            }
        ));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(kTestMonitorWidth / 16));
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(1).Data().at(15), "2");
    EXPECT_EQ(entries.at(1).Data().at(2), evget::FromDouble(kTestMonitorWidth / 8));
}

TEST(EvgetWindowsTransformer, TouchScreenColumnMapsToMonitor) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, Screen()).WillRepeatedly(Return(std::optional{std::string{kTestPointerDisplay}}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidReportFrom({MakeContact(1, 0, 0)})}));

    const auto report = MakeHidReportBytes(8);

    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    evgetwindows::EventTransformer touchscreen{query, hid_query, tracker};
    auto screen_data = touchscreen.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    ASSERT_FALSE(screen_data.Entries().empty());
    EXPECT_EQ(screen_data.Entries().at(0).Data().at(10), std::string{kTestMappedDisplay});

    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchpad));
    evgetwindows::EventTransformer touchpad{query, hid_query, tracker};
    auto pad_data = touchpad.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    ASSERT_FALSE(pad_data.Entries().empty());
    EXPECT_EQ(pad_data.Entries().at(0).Data().at(10), std::string{kTestPointerDisplay});
}

TEST(EvgetWindowsTransformer, TouchMotionScalesMonitor) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_))
        .WillOnce(Return(std::optional{MakeMappedMonitor()}))
        .WillRepeatedly(Return(std::optional{MakeMappedMonitorExtent(kTestMonitorWidth / 2, kTestMonitorHeight / 2)}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, kTestAxisMax / 16, 0)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto motion = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = motion.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(kTestMonitorWidth / 2 / 16));
}

TEST(EvgetWindowsTransformer, TouchMotionWithoutAxisRange) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::nullopt));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidReportFrom({MakeContact(1, kTestAxisMax / 16, 0)})}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto second = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    ASSERT_EQ(second.Entries().size(), 1);
    EXPECT_EQ(second.Entries().at(0).Data().at(2), "");
    EXPECT_EQ(second.Entries().at(0).Data().at(3), "");

    auto third = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    ASSERT_EQ(third.Entries().size(), 1);
    EXPECT_EQ(third.Entries().at(0).Data().at(2), "");
    EXPECT_EQ(third.Entries().at(0).Data().at(3), "");
}

TEST(EvgetWindowsTransformer, TouchMotionWithoutMonitor) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::nullopt));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidReportFrom({MakeContact(1, kTestAxisMax / 16, 0)})}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto second = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    ASSERT_EQ(second.Entries().size(), 1);
    EXPECT_EQ(second.Entries().at(0).Data().at(2), "");
    EXPECT_EQ(second.Entries().at(0).Data().at(3), "");

    auto third = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    ASSERT_EQ(third.Entries().size(), 1);
    EXPECT_EQ(third.Entries().at(0).Data().at(2), "");
    EXPECT_EQ(third.Entries().at(0).Data().at(3), "");
}

TEST(EvgetWindowsTransformer, TouchContactUpMoveAndRelease) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidReport(7, true, true)}))
        .WillOnce(Return(std::optional{MakeHidReport(7, false, true)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), "");
    EXPECT_EQ(entries.at(0).Data().at(3), "");
    EXPECT_EQ(entries.at(0).Data().at(15), "7");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(15), "7");
    EXPECT_EQ(entries.at(1).Data().at(16), "");
    EXPECT_EQ(entries.at(1).Data().at(17), "");
    EXPECT_EQ(entries.at(1).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchContactMissingIsReleased) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0), MakeContact(2, 0, 0)}, 2)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 3);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(1).Data().at(15), "2");
    EXPECT_EQ(entries.at(2).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(2).Data().at(15), "2");
    EXPECT_EQ(entries.at(2).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchContactNoConfidenceIsReleased) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(3, 0, 0, true, true)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(3, 0, 0, true, false)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), "");
    EXPECT_EQ(entries.at(0).Data().at(15), "3");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(15), "3");
    EXPECT_EQ(entries.at(1).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchContactWithConfidencePress) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(3, 0, 0, true, true)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(3, 0, 0, true, false)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(3, 0, 0, true, true)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto rejected = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    auto regained = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    ASSERT_EQ(rejected.Entries().size(), 2);
    EXPECT_EQ(rejected.Entries().at(1).Data().at(18), "1");
    EXPECT_TRUE(regained.Entries().empty());
}

TEST(EvgetWindowsTransformer, TouchNoConfidentContactNoPress) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(5, 0, 0, true, false)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(5, 0, 0, true, true)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto first = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    auto second = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    EXPECT_TRUE(first.Entries().empty());
    EXPECT_TRUE(second.Entries().empty());
}

TEST(EvgetWindowsTransformer, TouchDeviceRemovalReleases) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0), MakeContact(2, 0, 0)}, 2)}));
    EXPECT_CALL(hid_query, RemoveDevice(HidDeviceHandle())).Times(1);

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeDeviceChangeRawEvent(HidDeviceHandle(), false)}
    );
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 4);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(14), "3");
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Data().at(18), "1");
    EXPECT_EQ(entries.at(2).Data().at(15), "2");
    EXPECT_EQ(entries.at(3).Data().at(15), "2");
    EXPECT_EQ(entries.at(3).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchFrameOnCompletion) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 2)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(2, 0, 0)}, 0)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto first = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    auto second = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    EXPECT_TRUE(first.Entries().empty());

    const auto& entries = second.Entries();
    ASSERT_EQ(entries.size(), 4);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Data().at(18), "0");
    EXPECT_EQ(entries.at(2).Data().at(15), "2");
    EXPECT_EQ(entries.at(3).Data().at(15), "2");
    EXPECT_EQ(entries.at(3).Data().at(18), "0");
}

TEST(EvgetWindowsTransformer, TouchIncompleteFrameCancelled) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 2)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(2, 0, 0)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto pending = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    EXPECT_TRUE(pending.Entries().empty());

    const auto& entries = data.Entries();
    ASSERT_EQ(entries.size(), 4);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Data().at(18), "1");
    EXPECT_EQ(entries.at(2).Data().at(15), "2");
    EXPECT_EQ(entries.at(3).Data().at(15), "2");
    EXPECT_EQ(entries.at(3).Data().at(18), "0");
}

TEST(EvgetWindowsTransformer, TouchpadButtonOneIsLeftClick) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchpad));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeContactlessReport(true)}))
        .WillOnce(Return(std::optional{MakeContactlessReport(false)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto press = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    auto release = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    ASSERT_EQ(press.Entries().size(), 1);
    EXPECT_EQ(press.Entries().at(0).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(press.Entries().at(0).Data().at(14), "2");
    EXPECT_EQ(press.Entries().at(0).Data().at(15), "");
    EXPECT_EQ(press.Entries().at(0).Data().at(16), std::to_string(0x110));
    EXPECT_EQ(press.Entries().at(0).Data().at(17), "BTN_LEFT");
    EXPECT_EQ(press.Entries().at(0).Data().at(18), "0");

    ASSERT_EQ(release.Entries().size(), 1);
    EXPECT_EQ(release.Entries().at(0).Data().at(15), "");
    EXPECT_EQ(release.Entries().at(0).Data().at(16), std::to_string(0x110));
    EXPECT_EQ(release.Entries().at(0).Data().at(17), "BTN_LEFT");
    EXPECT_EQ(release.Entries().at(0).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchpadRemovalReleasesButton) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchpad));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeContactlessReport(true)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeDeviceChangeRawEvent(HidDeviceHandle(), false)}
    );
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(0).Data().at(16), std::to_string(0x110));
    EXPECT_EQ(entries.at(0).Data().at(17), "BTN_LEFT");
    EXPECT_EQ(entries.at(0).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchpadContactlessReportAndButton) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchpad));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(1, 0, 0, false, true)}, 1)}))
        .WillOnce(Return(std::optional{MakeContactlessReport(true)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(0).Data().at(16), std::to_string(0x110));
    EXPECT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(EvgetWindowsTransformer, TouchZeroContacteleases) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({}, 0)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchIdenticalCoordinatesIndependent) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0), MakeContact(2, 0, 0)}, 2)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0), MakeContact(2, kTestAxisMax / 16, 0)}, 2)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto down = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    auto motion = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    ASSERT_EQ(down.Entries().size(), 4);
    EXPECT_EQ(down.Entries().at(0).Data().at(15), "1");
    EXPECT_EQ(down.Entries().at(1).Data().at(15), "1");
    EXPECT_EQ(down.Entries().at(2).Data().at(15), "2");
    EXPECT_EQ(down.Entries().at(3).Data().at(15), "2");

    const auto& entries = motion.Entries();
    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(0.0));
    EXPECT_EQ(entries.at(1).Data().at(15), "2");
    EXPECT_EQ(entries.at(1).Data().at(2), evget::FromDouble(kTestMonitorWidth / 16));
}

TEST(EvgetWindowsTransformer, TouchDuplicateContactOneRow) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0), MakeContact(1, 0, 0)}, 2)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(15), "1");
}

TEST(EvgetWindowsTransformer, TouchDuplicateKeepsLast) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}))
        .WillOnce(Return(
            std::optional{MakeHidFrame({MakeContact(1, kTestAxisMax / 16, 0), MakeContact(1, kTestAxisMax / 4, 0)}, 2)}
        ));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    static_cast<void>(transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)}));
    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(kTestMonitorWidth / 4));
}

TEST(EvgetWindowsTransformer, TouchFrameRowsAreOrdered) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(
            Return(std::optional{MakeHidFrame({MakeContact(3, 0, 0), MakeContact(1, 0, 0), MakeContact(2, 0, 0)}, 3)})
        );

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.size(), 6);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(15), "1");
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(15), "1");
    EXPECT_EQ(entries.at(2).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(2).Data().at(15), "2");
    EXPECT_EQ(entries.at(3).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(3).Data().at(15), "2");
    EXPECT_EQ(entries.at(4).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(4).Data().at(15), "3");
    EXPECT_EQ(entries.at(5).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(5).Data().at(15), "3");
}

TEST(EvgetWindowsTransformer, TouchRepeatedDownChangesZero) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, AxisRange(testing::_)).WillRepeatedly(Return(std::optional{MakeAxisRange()}));
    EXPECT_CALL(query, MappedMonitor(testing::_)).WillRepeatedly(Return(std::optional{MakeMappedMonitor()}));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidFrame({MakeContact(4, kTestAxisMax / 2, kTestAxisMax / 2)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto down = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});
    auto replay = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    ASSERT_EQ(down.Entries().size(), 2);

    const auto& entries = replay.Entries();
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(entries.at(0).Data().at(2), evget::FromDouble(0.0));
    EXPECT_EQ(entries.at(0).Data().at(3), evget::FromDouble(0.0));
}

TEST(EvgetWindowsTransformer, TouchTwoDevicesTrackedIndependently) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(query, DeviceName(HidDeviceHandle())).WillRepeatedly(Return(std::optional<std::string>{"touch-first"}));
    EXPECT_CALL(query, DeviceName(HidDeviceHandleAlternate()))
        .WillRepeatedly(Return(std::optional<std::string>{"touch-second"}));
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}))
        .WillOnce(Return(std::optional{MakeHidFrame({MakeContactState(1, 0, 0, false, true)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto first_down = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEventFrom(HidDeviceHandle(), report)}
    );
    auto second_down = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEventFrom(HidDeviceHandleAlternate(), report)}
    );
    auto first_up = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEventFrom(HidDeviceHandle(), report)}
    );

    ASSERT_EQ(first_down.Entries().size(), 2);
    EXPECT_EQ(first_down.Entries().at(0).Data().at(4), "touch-first");

    // The second device's contact one is its own, so it presses rather than reading as already tracked.
    ASSERT_EQ(second_down.Entries().size(), 2);
    EXPECT_EQ(second_down.Entries().at(0).Data().at(4), "touch-second");
    EXPECT_EQ(second_down.Entries().at(1).Data().at(18), "0");

    ASSERT_EQ(first_up.Entries().size(), 2);
    EXPECT_EQ(first_up.Entries().at(0).Data().at(4), "touch-first");
    EXPECT_EQ(first_up.Entries().at(1).Data().at(18), "1");
}

TEST(EvgetWindowsTransformer, TouchDeviceStateHasBound) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidFrame({MakeContact(1, 0, 0)}, 1)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    std::size_t held_rows = 0;
    for (std::size_t index = 0; index < evgetwindows::kMaxCachedDevices; ++index) {
        const auto batch = transformer.TransformEvent(
            evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEventFrom(HidDeviceHandleAt(index), report)}
        );
        held_rows += batch.Entries().size();
    }
    ASSERT_EQ(held_rows, 2 * evgetwindows::kMaxCachedDevices);

    auto data = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{
            MakeHidRawEventFrom(HidDeviceHandleAt(evgetwindows::kMaxCachedDevices), report)
        }
    );
    const auto& entries = data.Entries();

    // Each held device is released as the ceiling clears the map, then the arriving device presses its contact.
    ASSERT_EQ(entries.size(), 2 * (evgetwindows::kMaxCachedDevices + 1));
    EXPECT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.at(1).Data().at(18), "1");
    EXPECT_EQ(entries.back().Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(entries.back().Data().at(18), "0");
}

// Mirrors the touch down, motion and up assertions the evgetlibinput transformer tests make.
TEST(EvgetWindowsTransformer, TouchParityWithLibInput) {
    ExpectTouchParitySequence(evget::DeviceType::kTouchscreen, "3");
    ExpectTouchParitySequence(evget::DeviceType::kTouchpad, "2");
}

TEST(EvgetWindowsTransformer, TouchRowsAddNoColumns) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};
    EXPECT_CALL(hid_query, ClassifyDevice(testing::_)).WillRepeatedly(Return(evget::DeviceType::kTouchscreen));
    EXPECT_CALL(hid_query, DecodeReport(testing::_, testing::_))
        .WillRepeatedly(Return(std::optional{MakeHidReport(7, true, true)}));

    evgetwindows::EventTransformer transformer{query, hid_query, tracker};
    const auto report = MakeHidReportBytes(8);

    auto mouse_move =
        transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeMouseMoveRelative(1, 1)});
    auto mouse_click = transformer.TransformEvent(
        evget::InputEvent<evgetwindows::RawEvent>{MakeMouseButton(RI_MOUSE_LEFT_BUTTON_DOWN)}
    );
    auto touch = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{MakeHidRawEvent(report)});

    ASSERT_EQ(mouse_move.Entries().size(), 1);
    ASSERT_EQ(mouse_click.Entries().size(), 1);
    ASSERT_EQ(touch.Entries().size(), 2);

    EXPECT_EQ(touch.Entries().at(0).Type(), evget::EntryType::kMouseMove);
    EXPECT_EQ(touch.Entries().at(0).Data().size(), mouse_move.Entries().at(0).Data().size());
    EXPECT_EQ(touch.Entries().at(1).Type(), evget::EntryType::kMouseClick);
    EXPECT_EQ(touch.Entries().at(1).Data().size(), mouse_click.Entries().at(0).Data().size());
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
