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
#include <utility>
#include <vector>

#include "common/windows_mock.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evget/input_event.h"
#include "evgetwindows/hid_frame.h"
#include "evgetwindows/message_window.h"
#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows_query_api.h"

using test::AsRawInput;
using test::HidQueryApiMock;
using test::kTestAxisMax;
using test::kTestMappedDisplay;
using test::kTestMonitorWidth;
using test::kTestPointerDisplay;
using test::MakeAxisRange;
using test::MakeContact;
using test::MakeDeviceChangeRawEvent;
using test::MakeHidPacket;
using test::MakeHidPacketNullDevice;
using test::MakeHidPacketUndersized;
using test::MakeHidRawEvent;
using test::MakeHidReport;
using test::MakeHidReportBytes;
using test::MakeHidReportFrom;
using test::MakeInjected;
using test::MakeKeyboard;
using test::MakeMappedMonitor;
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
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet));
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
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet));
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
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet));
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
    const auto raw_event = evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet));
    ASSERT_TRUE(raw_event.has_value());

    auto data = transformer.TransformEvent(evget::InputEvent<evgetwindows::RawEvent>{*raw_event});

    EXPECT_EQ(data.Entries().size(), 0);
}

TEST(EvgetWindowsTransformer, TouchInvalidReport) {
    const auto report = MakeHidReportBytes(evgetwindows::kHidReportCapacity + 1);
    const auto packet = MakeHidPacket(report, 1);

    EXPECT_FALSE(evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet)).has_value());
}

TEST(EvgetWindowsTransformer, TouchInconsistentPackets) {
    const auto report = MakeHidReportBytes(8);
    const auto packet = MakeHidPacketUndersized(report, 1);

    EXPECT_FALSE(evgetwindows::MessageWindow::ToRawEvent(AsRawInput(packet)).has_value());
}

TEST(EvgetWindowsTransformer, DeviceRemovalRemovesHid) {
    NiceMock<WindowsQueryApiMock> query{};
    NiceMock<HidQueryApiMock> hid_query{};
    evgetwindows::ModifierTracker tracker{};

    int backing = 0;
    HANDLE device = &backing;
    EXPECT_CALL(hid_query, EvictDevice(device)).Times(1);

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
    EXPECT_CALL(hid_query, EvictDevice(testing::_)).Times(0);

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

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
