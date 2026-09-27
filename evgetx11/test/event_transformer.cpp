#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <evget/event/device_type.h>
#include <evget/event/entry.h>
#include <evget/event/schema.h>
#include <evgetx11/input_event.h>

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "common/x11_mock.h"
#include "evgetx11/event_switch.h"
#include "evgetx11/event_switch_pointer_key.h"
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

    test::SetXWrapperDeviceMocks(x_wrapper_mock, devices.front(), xi_devices.front(), device_type);

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
    XDeviceInfo device{
        .id = 1,
        .type = 1,
        .name = device_name.data(),
        .num_classes = 0,
        .use = IsXExtensionPointer,
        .inputclassinfo = nullptr
    };
    auto xi_device = test::CreateXiDeviceInfo(any_class_info, device_name);
    test::SetXWrapperDeviceMocks(x_wrapper_mock, device, xi_device, device_type);

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

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
