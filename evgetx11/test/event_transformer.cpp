#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <evget/event/device_type.h>
#include <evget/event/entry.h>
#include <evget/event/schema.h>
#include <evgetx11/input_event.h>

#include <array>
#include <utility>

#include "common/x11_api_mock.h"
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
    auto transformer = evgetx11::EventTransformer{x_wrapper_mock, x_event_switch, x_event_switch_pointer_key};

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

    x_event_switch_pointer_key.RefreshDevices(1, 1, evget::DeviceType::kMouse, "name", xi_device_info, x_event_switch);
    auto input_event = evgetx11::InputEvent::NextEvent(x_wrapper_mock);

    auto data = transformer.TransformEvent(std::move(input_event));
    const auto& entries = data.Entries();

    ASSERT_EQ(entries.at(0).Type(), evget::EntryType::kMouseClick);
    ASSERT_EQ(entries.at(0).Data().at(0), "");
    ASSERT_EQ(entries.at(0).Data().at(2), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(3), evget::FromDouble(1.0));
    ASSERT_EQ(entries.at(0).Data().at(4), "name");
    ASSERT_EQ(entries.at(0).Data().at(11), "0");
    ASSERT_EQ(entries.at(0).Data().at(12), "0");
    ASSERT_EQ(entries.at(0).Data().at(13), "MOUSE");
    ASSERT_EQ(entries.at(0).Data().at(14), "0");
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
