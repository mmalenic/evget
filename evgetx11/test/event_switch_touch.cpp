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

#include "common/x11_api_mock.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evgetx11/event_switch.h"
// clang-format off
#include "evgetx11/event_switch_touch.h"
// clang-format on
#include "evgetx11/input_event.h"
#include "evgetx11/x11_api.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(XEventSwitchTouchTest, TestTouchBegin) {  // NOLINT(readability-function-cognitive-complexity)
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchTouch x_event_switch_touch{};  // NOLINT(misc-const-correctness)

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto button_class_info = test::CreateXiButtonClassInfo(labels, mask);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&button_class_info)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    char name[] = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawTouchBegin, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    EXPECT_CALL(x_wrapper_mock, EventData)
        .WillOnce(
            testing::Return(testing::ByMove<evgetx11::XEventPointer>({&x_event.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(testing::ByMove(test::CreateXEvent(device_event))));
    EXPECT_CALL(x_wrapper_mock, GetDeviceButtonMapping)
        .WillOnce(
            testing::Return(testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1)))
        );
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);

    auto data = evget::Data{};
    x_event_switch_touch.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(0), "1");
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_EQ(entries.at(0).Data().at(11), "0");

    ASSERT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(1).Data().at(0), "1");
    ASSERT_EQ(entries.at(1).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(1).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(1).Data().at(4), "name");
    ASSERT_EQ(entries.at(1).Data().at(11), "0");
    ASSERT_EQ(entries.at(1).Data().at(12), "0");
    ASSERT_EQ(entries.at(1).Data().at(13), "TOUCHSCREEN");
    ASSERT_EQ(entries.at(1).Data().at(14), "0");
}

TEST(XEventSwitchTouchTest, TestTouchUpdate) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchTouch x_event_switch_touch{};

    auto valuator_class_info = test::CreateXiValuatorClassInfo();
    valuator_class_info.number = 0;

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&valuator_class_info)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    char name[] = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawTouchUpdate, valuator_mask, values);

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
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);

    auto data = evget::Data{};
    x_event_switch_touch.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
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

TEST(XEventSwitchTouchTest, TestTouchEnd) {  // NOLINT(readability-function-cognitive-complexity)
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchTouch x_event_switch_touch{};

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto button_class_info = test::CreateXiButtonClassInfo(labels, mask);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    std::array<XIAnyClassInfo*, 3> any_class_info = {reinterpret_cast<XIAnyClassInfo*>(&button_class_info)};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    char name[] = "name";
    auto xi_device_info = test::CreateXiDeviceInfo(any_class_info, name);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawTouchEnd, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    EXPECT_CALL(x_wrapper_mock, EventData)
        .WillOnce(
            testing::Return(testing::ByMove<evgetx11::XEventPointer>({&x_event.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(testing::ByMove(test::CreateXEvent(device_event))));
    EXPECT_CALL(x_wrapper_mock, GetDeviceButtonMapping)
        .WillOnce(
            testing::Return(testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1)))
        );
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly([]() { return std::nullopt; });
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);

    auto data = evget::Data{};
    x_event_switch_touch.SwitchOnEvent(input_event, data, x_event_switch, [](Time) {
        return std::optional{std::chrono::microseconds{1}};
    });

    auto entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(0).Data().at(0), "1");
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_EQ(entries.at(0).Data().at(11), "0");

    ASSERT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(1).Data().at(0), "1");
    ASSERT_EQ(entries.at(1).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(1).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(1).Data().at(4), "name");
    ASSERT_EQ(entries.at(1).Data().at(11), "0");
    ASSERT_EQ(entries.at(1).Data().at(12), "0");
    ASSERT_EQ(entries.at(1).Data().at(13), "TOUCHSCREEN");
    ASSERT_EQ(entries.at(1).Data().at(14), "1");
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
