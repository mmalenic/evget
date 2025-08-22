#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>

#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>

#include "evget/event/button_action.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/entry.h"
#include "evget/event/schema.h"
#include "evgetx11/XEventSwitch.h"
#include "utils/EvgetX11TestUtils.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(XEventSwitchTest, RefreshDevices) {
    test::XWrapperMock x_wrapper_mock{};
    evgetx11::XEventSwitch x_event_switch{x_wrapper_mock};
    ASSERT_FALSE(x_event_switch.HasDevice(1));

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", {});
    ASSERT_TRUE(x_event_switch.HasDevice(1));
}

TEST(XEventSwitchTest, GetDevice) {
    test::XWrapperMock x_wrapper_mock{};
    evgetx11::XEventSwitch x_event_switch{x_wrapper_mock};
    ASSERT_THROW(static_cast<void>(x_event_switch.GetDevice(1)), std::out_of_range);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", {});
    ASSERT_EQ(x_event_switch.GetDevice(1), evget::DeviceType::kMouse);
}

TEST(XEventSwitchTest, GetButtonName) {
    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto button_class_info = test::CreateXiButtonClassInfo(labels, mask);

    test::XWrapperMock x_wrapper_mock{};
    evgetx11::XEventSwitch x_event_switch{x_wrapper_mock};

    ASSERT_THROW(static_cast<void>(x_event_switch.GetButtonName(1, 0)), std::out_of_range);

    EXPECT_CALL(x_wrapper_mock, GetDeviceButtonMapping)
        .WillOnce(
            testing::Return(testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1)))
        );
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", {});
    x_event_switch.SetButtonMap(button_class_info, 1);
    ASSERT_EQ(x_event_switch.GetButtonName(1, 0), "MOUSE");
}

TEST(XEventSwitchPointerTest, TestAddButtonEvent) {  // NOLINT(readability-function-cognitive-complexity)
    test::XWrapperMock x_wrapper_mock{};
    evgetx11::XEventSwitch x_event_switch{x_wrapper_mock};

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);
    device_event.deviceid = 1;

    std::array<Atom, 1> labels = {1};
    std::array<unsigned char, 1> mask = {1};
    auto button_class_info = test::CreateXiButtonClassInfo(labels, mask);

    EXPECT_CALL(x_wrapper_mock, GetDeviceButtonMapping)
        .WillOnce(
            testing::Return(testing::ByMove<std::unique_ptr<unsigned char[]>>(std::make_unique<unsigned char[]>(1)))
        );
    EXPECT_CALL(x_wrapper_mock, AtomName)
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, [](void*) {
                                                                                                  return 0;
                                                                                              }})));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    x_event_switch.SetButtonMap(button_class_info, 1);
    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", {});

    evget::Data data{};  // NOLINT(misc-const-correctness)
    x_event_switch.AddButtonEvent(device_event, evget::TimestampType{}, data, evget::ButtonAction::kPress, 0, [](Time) {
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

TEST(XEventSwitchPointerTest, TestAddMotionEvent) {
    test::XWrapperMock x_wrapper_mock{};
    evgetx11::XEventSwitch x_event_switch{x_wrapper_mock};

    auto valuator_class_info = test::CreateXiValuatorClassInfo();
    valuator_class_info.number = 0;

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawMotion, valuator_mask, values);

    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", {});

    evget::Data data{};  // NOLINT(misc-const-correctness)
    x_event_switch.AddMotionEvent(device_event, evget::TimestampType{}, data, [](Time) {
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

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
