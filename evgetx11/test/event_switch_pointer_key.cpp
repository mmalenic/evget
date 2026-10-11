#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <xorg/xserver-properties.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <string>

#include "common/x11_mock.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evgetx11/event_switch.h"
// clang-format off
#include "evgetx11/event_switch_pointer_key.h"
// clang-format on
#include "evgetx11/input_event.h"
#include "evgetx11/x11.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(XEventSwitchPointerKeyTest, TestRefreshDevices) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    XIValuatorClassInfo valuator_info{
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 0,
        .label = 1,
        .min = 0,
        .max = 0,
        .value = 0,
        .resolution = 0,
        .mode = 0
    };
    XIScrollClassInfo scroll_info{
        .type = XIScrollClass,
        .sourceid = 0,
        .number = 0,
        .scroll_type = XIScrollTypeVertical,
        .increment = 0,
        .flags = 0
    };
    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {0};
    XISetMask(mask.data(), 1);
    XIButtonClassInfo button_info{
        .type = XIButtonClass,
        .sourceid = 0,
        .num_buttons = 1,
        .labels = labels.data(),
        .state = XIButtonState{
            .mask_len = 1,
            .mask = mask.data(),
        },
    };

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* any_valuator_info = reinterpret_cast<XIAnyClassInfo*>(&valuator_info);
    auto* any_scroll_info = reinterpret_cast<XIAnyClassInfo*>(&scroll_info);
    auto* any_button_info = reinterpret_cast<XIAnyClassInfo*>(&button_info);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array classes = {any_valuator_info, any_scroll_info, any_button_info};

    std::string name = "name";
    const XIDeviceInfo xi2_device_info{
        .deviceid = 0,
        .name = name.data(),
        .use = 0,
        .attachment = 0,
        .enabled = True,
        .num_classes = 1,
        .classes = classes.data()
    };

    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));

    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi2_device_info, x_event_switch);
}

TEST(XEventSwitchPointerKeyTest, RefreshDeviceSkipsValuators) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    XIValuatorClassInfo valuator_info{
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 0,
        .label = None,
        .min = 0,
        .max = 0,
        .value = 0,
        .resolution = 0,
        .mode = 0
    };
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array classes = {reinterpret_cast<XIAnyClassInfo*>(&valuator_info)};
    std::string name = "name";
    const XIDeviceInfo xi2_device_info{
        .deviceid = 0,
        .name = name.data(),
        .use = 0,
        .attachment = 0,
        .enabled = True,
        .num_classes = 1,
        .classes = classes.data()
    };

    EXPECT_CALL(x_wrapper_mock, AtomName).Times(0);

    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi2_device_info, x_event_switch);
}

TEST(XEventSwitchPointerKeyTest, TestButtonEvent) { // NOLINT(readability-function-cognitive-complexity)
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto button_class_info = test::CreateXiButtonClassInfo(labels, mask);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&button_class_info)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    test::SetXWrapperEventMocks(x_wrapper_mock, device_event, x_event);

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(0), "1");
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_TRUE(entries.at(0).Data().at(11).has_value());
    ASSERT_EQ(entries.at(0).Data().at(12), "XI_RawButtonPress");
    ASSERT_EQ(entries.at(0).Data().at(13), "x11");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
    ASSERT_FALSE(entries.at(0).Data().at(15).has_value());
    ASSERT_EQ(entries.at(0).Data().at(16), "0");
    ASSERT_FALSE(entries.at(0).Data().at(17).has_value());
    ASSERT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(XEventSwitchPointerKeyTest, ButtonEventUnregisteredDevice) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(test::CreateXEvent(device_event)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    auto data = evget::Data{};
    ASSERT_NO_THROW(
        static_cast<void>(x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
            return std::optional{std::chrono::microseconds{1}};
        }))
    );
    ASSERT_TRUE(data.Entries().empty());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(XEventSwitchPointerKeyTest, ButtonEventUnlabelled) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    std::array<Atom, 1> labels = {None};
    std::array<unsigned char, 1> mask = {1};
    auto button_class_info = test::CreateXiButtonClassInfo(labels, mask);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&button_class_info)};
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(test::CreateXEvent(device_event)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    EXPECT_CALL(x_wrapper_mock, AtomName).Times(0);
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([] { return test::CreatePointerResult(); });

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);
    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    auto data = evget::Data{};
    ASSERT_NO_THROW(
        static_cast<void>(x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
            return std::optional{std::chrono::microseconds{1}};
        }))
    );
    const auto& entries = data.Entries();
    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(16), "0");
    ASSERT_FALSE(entries.at(0).Data().at(17).has_value());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(XEventSwitchPointerKeyTest, WheelButtonWithoutlValuators) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto button_class_info = test::CreateXiButtonClassInfo(labels, mask);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&button_class_info)};
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {0};
    std::array<double, 1> values = {0};
    auto press = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);
    auto release = test::CreateXiRawEvent(XI_RawButtonRelease, valuator_mask, values);
    press.detail = 1;
    release.detail = 1;
    EXPECT_CALL(x_wrapper_mock, NextEvent)
        .WillOnce(testing::Return(test::CreateXEvent(press)))
        .WillOnce(testing::Return(test::CreateXEvent(release)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    std::string label = BTN_LABEL_PROP_BTN_WHEEL_UP;
    EXPECT_CALL(x_wrapper_mock, AtomName).WillOnce([&label](Atom) {
        return std::unique_ptr<char[], decltype(&XFree)>{label.data(), [](void*) { return 0; }};
    });
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([] { return test::CreatePointerResult(); });

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    for (auto i = 0; i < 2; i++) {
        auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);
        static_cast<void>(x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
            return std::optional{std::chrono::microseconds{1}};
        }));
    }

    const auto& entries = data.Entries();
    ASSERT_EQ(entries.size(), 1);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_EQ(entries.at(0).Data().at(12), "XI_RawButtonPress");
    ASSERT_EQ(entries.at(0).Data().at(15), evget::FromDouble(-1.0));
    ASSERT_FALSE(entries.at(0).Data().at(16).has_value());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(XEventSwitchPointerKeyTest, RawButtonsAreNamedByPhysicalButton) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    constexpr int kLeft = 1;
    constexpr int kRight = 3;
    constexpr int kWheelUp = 4;
    constexpr int kWheelDown = 5;
    // Labels come in physical order whatever the button mapping, e.g. a left-handed map with natural scrolling.
    std::array names = {
        std::string{BTN_LABEL_PROP_BTN_LEFT},
        std::string{BTN_LABEL_PROP_BTN_MIDDLE},
        std::string{BTN_LABEL_PROP_BTN_RIGHT},
        std::string{BTN_LABEL_PROP_BTN_WHEEL_UP},
        std::string{BTN_LABEL_PROP_BTN_WHEEL_DOWN},
    };
    std::array<Atom, names.size()> labels{};
    std::ranges::iota(labels, Atom{1});
    std::array<unsigned char, 1> mask = {0};
    XIButtonClassInfo button_class_info{
        .type = XIButtonClass,
        .sourceid = 1,
        .num_buttons = static_cast<int>(labels.size()),
        .labels = labels.data(),
        .state = {.mask_len = static_cast<int>(mask.size()), .mask = mask.data()},
    };
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&button_class_info)};
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {0};
    std::array<double, 1> values = {0};
    std::array<XIRawEvent, 4> presses{};
    for (auto [press, detail] : std::views::zip(presses, std::array{kLeft, kRight, kWheelUp, kWheelDown})) {
        press = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);
        press.detail = detail;
    }
    EXPECT_CALL(x_wrapper_mock, NextEvent)
        .WillOnce(testing::Return(test::CreateXEvent(presses.at(0))))
        .WillOnce(testing::Return(test::CreateXEvent(presses.at(1))))
        .WillOnce(testing::Return(test::CreateXEvent(presses.at(2))))
        .WillOnce(testing::Return(test::CreateXEvent(presses.at(3))));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    EXPECT_CALL(x_wrapper_mock, AtomName).WillRepeatedly([&names](Atom atom) {
        return std::unique_ptr<char[], decltype(&XFree)>{names.at(atom - 1).data(), [](void*) { return 0; }};
    });
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([] { return test::CreatePointerResult(); });

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    for (std::size_t i = 0; i < presses.size(); i++) {
        auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);
        static_cast<void>(x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
            return std::optional{std::chrono::microseconds{1}};
        }));
    }

    const auto& entries = data.Entries();
    ASSERT_EQ(entries.size(), 4);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(16), std::to_string(kLeft));
    ASSERT_EQ(entries.at(0).Data().at(17), BTN_LABEL_PROP_BTN_LEFT);
    ASSERT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(1).Data().at(16), std::to_string(kRight));
    ASSERT_EQ(entries.at(1).Data().at(17), BTN_LABEL_PROP_BTN_RIGHT);
    ASSERT_EQ(entries.at(2).Type(), evget::EntryType::kMouseScroll);
    ASSERT_EQ(entries.at(2).Data().at(15), evget::FromDouble(-1.0));
    ASSERT_EQ(entries.at(3).Type(), evget::EntryType::kMouseScroll);
    ASSERT_EQ(entries.at(3).Data().at(15), evget::FromDouble(1.0));
}

TEST(XEventSwitchCoreTest, TestKeyEvent) { // NOLINT(readability-function-cognitive-complexity)
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    std::array<XIAnyClassInfo*, 3> any_class_info = {};
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawKeyPress, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    test::SetXWrapperKeyMocks(x_wrapper_mock, device_event, x_event);
    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kKeyboard, "name", xi_device_info);
    x_event_switch_pointer_key
        .RefreshDevices(1, 1, evget::DeviceType::kKeyboard, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    ASSERT_EQ(entries.at(0).Data().at(0), "1");
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_TRUE(entries.at(0).Data().at(11).has_value());
    ASSERT_EQ(entries.at(0).Data().at(12), "XI_RawKeyPress");
    ASSERT_EQ(entries.at(0).Data().at(13), "x11");
    ASSERT_EQ(entries.at(0).Data().at(14), "1");
    ASSERT_EQ(entries.at(0).Data().at(15), "0");
    ASSERT_EQ(entries.at(0).Data().at(16), "a");
    ASSERT_EQ(entries.at(0).Data().at(17), "a");
    ASSERT_EQ(entries.at(0).Data().at(18), "0");
}

TEST(XEventSwitchCoreTest, KeyReleaseStoredCorrectly) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    std::array<XIAnyClassInfo*, 3> any_class_info = {};
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawKeyRelease, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    test::SetXWrapperKeyMocks(x_wrapper_mock, device_event, x_event);
    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kKeyboard, "name", xi_device_info);
    x_event_switch_pointer_key
        .RefreshDevices(1, 1, evget::DeviceType::kKeyboard, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kKey);
    ASSERT_EQ(entries.at(0).Data().at(12), "XI_RawKeyRelease");
    ASSERT_EQ(entries.at(0).Data().at(18), "1");
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(XEventSwitchCoreTest, TestMotionEvent) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    auto valuator_class_info = test::CreateXiValuatorClassInfo();
    valuator_class_info.number = 0;

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&valuator_class_info)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawMotion, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    EXPECT_CALL(x_wrapper_mock, EventData)
        .WillOnce(
            testing::Return(testing::ByMove<evgetx11::XEventPointer>({&x_event.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(testing::ByMove(test::CreateXEvent(device_event))));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(
            testing::Return(
                testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>(
                    {const_cast<char*>(AXIS_LABEL_PROP_ABS_X), [](void*) { return 0; }}
                )
            )
        );
    // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(0), "1");
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_TRUE(entries.at(0).Data().at(11).has_value());
    ASSERT_EQ(entries.at(0).Data().at(12), "XI_RawMotion");
    ASSERT_EQ(entries.at(0).Data().at(13), "x11");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
}

TEST(XEventSwitchCoreTest, RefreshDevicesScrollAxis) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    auto valuator_class_info = test::CreateXiValuatorClassInfo();
    auto scroll_class_info = test::CreateXiScrollClassInfo();
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> scroll_classes = {
        reinterpret_cast<XIAnyClassInfo*>(&valuator_class_info),
        reinterpret_cast<XIAnyClassInfo*>(&scroll_class_info)
    };
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> no_classes = {};
    std::string name = "name";
    auto scroll_device = test::CreateXiDeviceInfo(scroll_classes, name);
    auto plain_device = test::CreateXiDeviceInfo(no_classes, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawMotion, valuator_mask, values);
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(test::CreateXEvent(device_event)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([] { return test::CreatePointerResult(); });

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", scroll_device);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", scroll_device, x_event_switch);
    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", plain_device);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", plain_device, x_event_switch);
    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    ASSERT_FALSE(std::ranges::any_of(data.Entries(), [](const evget::Entry& entry) {
        return entry.Type() == evget::EntryType::kMouseScroll;
    }));
}

TEST(XEventSwitchCoreTest, TestScrollEvent) { // NOLINT(readability-function-cognitive-complexity)
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    auto valuator_class_info = test::CreateXiValuatorClassInfo();
    auto scroll_class_info = test::CreateXiScrollClassInfo();

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {
        reinterpret_cast<XIAnyClassInfo*>(&valuator_class_info),
        reinterpret_cast<XIAnyClassInfo*>(&scroll_class_info)
    };
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawMotion, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    EXPECT_CALL(x_wrapper_mock, EventData)
        .WillOnce(
            testing::Return(testing::ByMove<evgetx11::XEventPointer>({&x_event.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(testing::ByMove(test::CreateXEvent(device_event))));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    ASSERT_EQ(entries.at(0).Data().at(0), "1");
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_TRUE(entries.at(0).Data().at(11).has_value());
    ASSERT_EQ(entries.at(0).Data().at(12), "XI_RawMotion");
    ASSERT_EQ(entries.at(0).Data().at(13), "x11");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
    ASSERT_EQ(entries.at(0).Data().at(15), evget::FromDouble(2.0));
    ASSERT_FALSE(entries.at(0).Data().at(16).has_value());
}

TEST(XEventSwitchCoreTest, TestScrollEventVerticalIncrement) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    auto valuator_class_info = test::CreateXiValuatorClassInfo();
    auto scroll_class_info = test::CreateXiScrollClassInfo();
    scroll_class_info.increment = -1;

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {
        reinterpret_cast<XIAnyClassInfo*>(&valuator_class_info),
        reinterpret_cast<XIAnyClassInfo*>(&scroll_class_info)
    };
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawMotion, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    EXPECT_CALL(x_wrapper_mock, EventData)
        .WillOnce(
            testing::Return(testing::ByMove<evgetx11::XEventPointer>({&x_event.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(testing::ByMove(test::CreateXEvent(device_event))));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    ASSERT_EQ(entries.at(0).Data().at(15), evget::FromDouble(-2.0));
    ASSERT_FALSE(entries.at(0).Data().at(16).has_value());
}

TEST(XEventSwitchCoreTest, TestScrollEventHorizontalIncrement) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    auto valuator_class_info = test::CreateXiValuatorClassInfo();
    auto scroll_class_info = test::CreateXiScrollClassInfo();
    scroll_class_info.scroll_type = XIScrollTypeHorizontal;
    scroll_class_info.increment = -1;

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {
        reinterpret_cast<XIAnyClassInfo*>(&valuator_class_info),
        reinterpret_cast<XIAnyClassInfo*>(&scroll_class_info)
    };
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    std::string name = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawMotion, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    EXPECT_CALL(x_wrapper_mock, EventData)
        .WillOnce(
            testing::Return(testing::ByMove<evgetx11::XEventPointer>({&x_event.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(testing::ByMove(test::CreateXEvent(device_event))));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);

    auto data = evget::Data{};
    x_event_switch_pointer_key.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseScroll);
    ASSERT_FALSE(entries.at(0).Data().at(15).has_value());
    ASSERT_EQ(entries.at(0).Data().at(16), evget::FromDouble(-2.0));
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
