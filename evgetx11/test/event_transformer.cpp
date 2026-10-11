#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <evget/device_id.h>
#include <evget/event/device_type.h>
#include <evget/event/entry.h>
#include <evget/event/schema.h>
#include <evget/event_transformer.h>
#include <evgetx11/input_event.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "common/log.h"
#include "common/x11_mock.h"
#include "evgetx11/event_switch.h"
#include "evgetx11/event_switch_pointer_key.h"
#include "evgetx11/log_once.h"
// clang-format off
#include "evgetx11/event_transformer.h"
// clang-format on

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
TEST(EventTransformer, TransformEvent) {
    test::X11ApiMock x_wrapper_mock{};
    evgetx11::EventSwitch x_event_switch{x_wrapper_mock};
    evgetx11::EventSwitchPointerKey x_event_switch_pointer_key{x_wrapper_mock};

    test::SetXWrapperMocks(x_wrapper_mock);

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
    auto device_event = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);

    auto x_event = test::CreateXEvent(device_event);
    SetXWrapperEventMocks(x_wrapper_mock, device_event, x_event);

    x_event_switch.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info);
    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);
    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    auto transformer = evgetx11::EventTransformer{x_wrapper_mock, x_event_switch, x_event_switch_pointer_key};
    auto data = transformer.TransformEvent(std::move(input_event));
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_FALSE(entries.at(0).Data().at(0).has_value());
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

TEST(EventTransformer, TouchEvents) {
    test::X11ApiMock x_wrapper_mock{};
    std::string device_name = "touchscreen";
    std::string device_type = XI_TOUCHSCREEN;
    std::array<XIAnyClassInfo*, 3> any_class_info = {};
    std::array devices{XDeviceInfo{
        .id = 1,
        .type = 1,
        .name = device_name.data(),
        .num_classes = 0,
        .use = IsXExtensionPointer,
        .inputclassinfo = nullptr
    }};
    std::array xi_devices{test::CreateXiDeviceInfo(any_class_info, device_name)};

    test::SetXWrapperDeviceMocks(x_wrapper_mock, devices, xi_devices, device_type);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto device_event = test::CreateXiRawEvent(XI_RawTouchBegin, valuator_mask, values);
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(test::CreateXEvent(device_event)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([] { return test::CreatePointerResult(); });

    auto transformer = std::move(evgetx11::EventTransformerBuilder{}.Touch()).Build(x_wrapper_mock);
    ASSERT_NE(transformer, nullptr);

    auto data = transformer->TransformEvent(evgetx11::InputEvent::NextEvent(x_wrapper_mock));
    const auto& entries = data.Entries();
    ASSERT_EQ(entries.size(), 2);
    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseMove);
    ASSERT_EQ(entries.at(1).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(1).Data().at(4), "touchscreen");
}

TEST(EventTransformer, RemovedDeviceNewUuid) {
    test::X11ApiMock x_wrapper_mock{};
    std::string device_name = "touchscreen";
    std::string device_type = XI_TOUCHSCREEN;
    std::array<XIAnyClassInfo*, 3> any_class_info = {};
    std::array devices{test::CreateXDeviceInfo(1, device_name)};
    std::array xi_devices{test::CreateXiDeviceInfo(any_class_info, device_name)};
    test::SetXWrapperDeviceMocks(x_wrapper_mock, devices, xi_devices, device_type);

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto touch = test::CreateXiRawEvent(XI_RawTouchUpdate, valuator_mask, values);
    std::array enabled_info{
        XIHierarchyInfo{.deviceid = 1, .attachment = 0, .use = 0, .enabled = True, .flags = XIDeviceEnabled}
    };
    std::array removed_info{
        XIHierarchyInfo{.deviceid = 1, .attachment = 0, .use = 0, .enabled = False, .flags = XISlaveRemoved}
    };
    auto enabled = test::CreateXiHierarchyEvent(enabled_info);
    auto removed = test::CreateXiHierarchyEvent(removed_info);

    EXPECT_CALL(x_wrapper_mock, NextEvent)
        .WillOnce(testing::Return(test::CreateXEvent(touch)))
        .WillOnce(testing::Return(test::CreateXEvent(enabled)))
        .WillOnce(testing::Return(test::CreateXEvent(touch)))
        .WillOnce(testing::Return(test::CreateXEvent(removed)))
        .WillOnce(testing::Return(test::CreateXEvent(touch)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([] { return test::CreatePointerResult(); });

    auto transformer = std::move(evgetx11::EventTransformerBuilder{}.Touch()).Build(x_wrapper_mock);
    ASSERT_NE(transformer, nullptr);
    auto device_uuid = [&transformer, &x_wrapper_mock] {
        constexpr std::size_t kDeviceIdColumn = 11;
        auto data = transformer->TransformEvent(evgetx11::InputEvent::NextEvent(x_wrapper_mock));
        return data.Entries().at(0).Data().at(kDeviceIdColumn);
    };

    const auto first = device_uuid();
    ASSERT_TRUE(transformer->TransformEvent(evgetx11::InputEvent::NextEvent(x_wrapper_mock)).Entries().empty());
    ASSERT_EQ(device_uuid(), first);
    ASSERT_TRUE(transformer->TransformEvent(evgetx11::InputEvent::NextEvent(x_wrapper_mock)).Entries().empty());
    ASSERT_NE(device_uuid(), first);
}

TEST(EventTransformer, WarnUnlistedDevicesCount) {
    const test::CapturedLog log{};
    evgetx11::LogOnce log_once{};
    std::string pointer = "pointer";
    std::string master = "master";
    std::array devices{test::CreateXDeviceInfo(1, pointer)};
    std::array xi_devices{test::CreateXiDeviceInfo(1, pointer), test::CreateXiDeviceInfo(2, master)};

    evgetx11::WarnUnlistedDevices(devices, xi_devices, log_once);

    ASSERT_TRUE(log.Lines().empty());
}

TEST(EventTransformer, WarnUnlistedDevicesLargeId) {
    const test::CapturedLog log{};
    evgetx11::LogOnce log_once{};
    std::string pointer = "pointer";
    std::string tablet = "tablet";
    constexpr int kLargeId = 200;
    std::array devices{test::CreateXDeviceInfo(1, pointer)};
    std::array xi_devices{test::CreateXiDeviceInfo(1, pointer), test::CreateXiDeviceInfo(kLargeId, tablet)};

    evgetx11::WarnUnlistedDevices(devices, xi_devices, log_once);
    evgetx11::WarnUnlistedDevices(devices, xi_devices, log_once);

    const std::string message =
        "device 'tablet' has ID 200, above the 127 that XInput 1 can list, so its events are not captured";
    ASSERT_EQ(log.Lines(), (std::vector<std::string>{"warning " + message, "debug " + message}));
}

TEST(EventTransformer, WarnUnlistedDevicesSmallId) {
    const test::CapturedLog log{};
    evgetx11::LogOnce log_once{};
    std::string pointer = "pointer";
    std::string tablet = "tablet";
    constexpr int kLargeId = 200;
    constexpr int kLargestXi1Id = 127;
    std::array devices{test::CreateXDeviceInfo(kLargeId, tablet)};
    std::array xi_devices{test::CreateXiDeviceInfo(kLargeId, tablet), test::CreateXiDeviceInfo(kLargestXi1Id, pointer)};

    evgetx11::WarnUnlistedDevices(devices, xi_devices, log_once);

    ASSERT_TRUE(log.Lines().empty());
}

TEST(EventTransformer, RefreshDevicesNoWarn) {
    const test::CapturedLog log{};
    test::X11ApiMock x_wrapper_mock{};
    std::string pointer = "pointer";
    std::string master = "master";
    std::string device_type = XI_MOUSE;
    std::array devices{test::CreateXDeviceInfo(1, pointer)};
    std::array xi_devices{test::CreateXiDeviceInfo(1, pointer), test::CreateXiDeviceInfo(2, master)};
    test::SetXWrapperDeviceMocks(x_wrapper_mock, devices, xi_devices, device_type);

    auto transformer = std::move(evgetx11::EventTransformerBuilder{}.Touch()).Build(x_wrapper_mock);

    ASSERT_NE(transformer, nullptr);
    ASSERT_FALSE(std::ranges::any_of(log.Lines(), [](const std::string& line) { return line.starts_with("warning"); }));
}

TEST(EventTransformer, RefreshDevicesSkipsRemoved) {
    test::X11ApiMock x_wrapper_mock{};
    std::string pointer = "pointer";
    std::string removed = "removed";
    std::string device_type = XI_MOUSE;
    std::array devices{test::CreateXDeviceInfo(1, pointer), test::CreateXDeviceInfo(2, removed)};
    std::array xi_devices{test::CreateXiDeviceInfo(1, pointer)};
    test::SetXWrapperDeviceMocks(x_wrapper_mock, devices, xi_devices, device_type);

    std::unique_ptr<evget::EventTransformer<evgetx11::InputEvent>> transformer{};
    ASSERT_NO_THROW(transformer = std::move(evgetx11::EventTransformerBuilder{}.Touch()).Build(x_wrapper_mock));
    ASSERT_NE(transformer, nullptr);
}

TEST(EventTransformer, InjectedDeviceType) {
    ASSERT_EQ(evgetx11::InjectedDeviceType(XISlaveKeyboard), evget::DeviceType::kKeyboard);
    ASSERT_EQ(evgetx11::InjectedDeviceType(XISlavePointer), evget::DeviceType::kMouse);
    ASSERT_EQ(evgetx11::InjectedDeviceType(XIMasterPointer), evget::DeviceType::kUnknown);
    ASSERT_EQ(evgetx11::InjectedDeviceType(XIFloatingSlave), evget::DeviceType::kUnknown);
}

TEST(EventTransformer, Xi1DeviceTypeKnown) {
    ASSERT_EQ(evgetx11::Xi1DeviceType(XI_MOUSE), evget::DeviceType::kMouse);
    ASSERT_EQ(evgetx11::Xi1DeviceType(XI_KEYBOARD), evget::DeviceType::kKeyboard);
    ASSERT_EQ(evgetx11::Xi1DeviceType("STYLUS"), evget::DeviceType::kTablet);
    ASSERT_EQ(evgetx11::Xi1DeviceType("JOYSTICK"), evget::DeviceType::kUnknown);
    ASSERT_EQ(evgetx11::Xi1DeviceType(nullptr), evget::DeviceType::kUnknown);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(EventTransformer, XTestDevicesAreInjected) {
    test::X11ApiMock x_wrapper_mock{};
    std::string xtest_pointer = "Virtual core XTEST pointer";
    std::string xtest_keyboard = "Virtual core XTEST keyboard";
    std::string real_mouse = "real mouse";
    std::string device_type = XI_MOUSE;
    constexpr int kXTestPointer = 4;
    constexpr int kXTestKeyboard = 5;
    constexpr int kRealMouse = 6;
    std::array devices{
        test::CreateXDeviceInfo(kXTestPointer, xtest_pointer),
        test::CreateXDeviceInfo(kXTestKeyboard, xtest_keyboard),
        test::CreateXDeviceInfo(kRealMouse, real_mouse)
    };
    devices.at(0).type = None;
    devices.at(1).type = None;
    std::array xi_devices{
        test::CreateXiDeviceInfo(kXTestPointer, xtest_pointer),
        test::CreateXiDeviceInfo(kXTestKeyboard, xtest_keyboard),
        test::CreateXiDeviceInfo(kRealMouse, real_mouse)
    };
    xi_devices.at(1).use = XISlaveKeyboard;
    test::SetXWrapperDeviceMocks(x_wrapper_mock, devices, xi_devices, device_type);
    EXPECT_CALL(x_wrapper_mock, IsXTestDevice).WillRepeatedly([](int device_id) {
        return device_id == kXTestPointer || device_id == kXTestKeyboard;
    });

    std::array<unsigned char, 1> valuator_mask = {1};
    std::array<double, 1> values = {1};
    auto key = test::CreateXiRawEvent(XI_RawKeyPress, valuator_mask, values);
    key.sourceid = kXTestKeyboard;
    auto injected_click = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);
    injected_click.sourceid = kXTestPointer;
    auto real_click = test::CreateXiRawEvent(XI_RawButtonPress, valuator_mask, values);
    real_click.sourceid = kRealMouse;
    EXPECT_CALL(x_wrapper_mock, NextEvent)
        .WillOnce(testing::Return(test::CreateXEvent(key)))
        .WillOnce(testing::Return(test::CreateXEvent(injected_click)))
        .WillOnce(testing::Return(test::CreateXEvent(real_click)));
    test::SetXWrapperInPlaceEventDataMocks(x_wrapper_mock);
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow).WillRepeatedly(testing::Return(std::nullopt));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([] { return test::CreatePointerResult(); });
    EXPECT_CALL(x_wrapper_mock, LookupCharacter).WillRepeatedly(testing::Return(std::string{}));

    auto transformer =
        std::move(evgetx11::EventTransformerBuilder{}.PointerKey(x_wrapper_mock).Touch()).Build(x_wrapper_mock);
    ASSERT_NE(transformer, nullptr);
    auto next = [&transformer, &x_wrapper_mock] {
        auto data = transformer->TransformEvent(evgetx11::InputEvent::NextEvent(x_wrapper_mock));
        return data.Entries().at(0);
    };
    evget::DeviceId<std::string> device_ids{};
    const auto& injected_uuid = device_ids.UuidDeterministic(std::string{"evget:x11:device:x11-injected"});
    constexpr std::size_t kDeviceName = 4;
    constexpr std::size_t kDeviceId = 11;
    constexpr std::size_t kDeviceType = 14;

    const auto key_entry = next();
    ASSERT_EQ(key_entry.Type(), evget::EntryType::kKey);
    ASSERT_EQ(key_entry.Data().at(kDeviceName), "x11-injected");
    ASSERT_EQ(key_entry.Data().at(kDeviceId), injected_uuid);
    ASSERT_EQ(key_entry.Data().at(kDeviceType), "1");

    const auto click_entry = next();
    ASSERT_EQ(click_entry.Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(click_entry.Data().at(kDeviceName), "x11-injected");
    ASSERT_EQ(click_entry.Data().at(kDeviceId), injected_uuid);
    ASSERT_EQ(click_entry.Data().at(kDeviceType), "0");

    const auto real_entry = next();
    ASSERT_EQ(real_entry.Data().at(kDeviceName), "real mouse");
    ASSERT_NE(real_entry.Data().at(kDeviceId), injected_uuid);
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
