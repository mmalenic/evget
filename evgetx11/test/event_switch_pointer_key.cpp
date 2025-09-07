#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <string>

#include "common/x11_api_mock.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evgetx11/event_switch.h"
// clang-format off
#include "evgetx11/event_switch_pointer_key.h"
// clang-format on
#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(XEventSwitchPointerKeyTest, TestRefreshDevices) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    XIValuatorClassInfo valuator_info{
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 0,
        .label = 0,
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
    ASSERT_EQ(entries.at(0).Data().at(11), "0");
    ASSERT_EQ(entries.at(0).Data().at(12), "0");
    ASSERT_EQ(entries.at(0).Data().at(13), "MOUSE");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
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
    ASSERT_EQ(entries.at(0).Data().at(11), "1");
    ASSERT_EQ(entries.at(0).Data().at(12), "0");
    ASSERT_EQ(entries.at(0).Data().at(13), "A");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
    ASSERT_EQ(entries.at(0).Data().at(15), "a");
}

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
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

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
    ASSERT_EQ(entries.at(0).Data().at(11), "0");
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
    ASSERT_EQ(entries.at(0).Data().at(11), "0");
    ASSERT_EQ(entries.at(0).Data().at(12), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(13), "");
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
