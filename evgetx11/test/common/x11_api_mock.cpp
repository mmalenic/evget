#include "common/x11_api_mock.h"

#include <gmock/gmock.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <evgetx11/x11_api.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
XIValuatorClassInfo test::CreateXiValuatorClassInfo() {
    return {
        .type = XIValuatorClass,
        .sourceid = 0,
        .number = 1,
        .label = 1,
        .min = 0,
        .max = 0,
        .value = 1,
        .resolution = 1,
        .mode = XIModeAbsolute
    };
}

XIScrollClassInfo test::CreateXiScrollClassInfo() {
    return {
        .type = XIScrollClass,
        .sourceid = 1,
        .number = 0,
        .scroll_type = XIScrollTypeVertical,
        .increment = 1,
        .flags = 0
    };
}

XIButtonClassInfo test::CreateXiButtonClassInfo(std::array<Atom, 1>& labels, std::array<unsigned char, 1>& mask) {
    return {
        .type = XIButtonClass,
        .sourceid = 1,
        .num_buttons = static_cast<int>(labels.size()),
        .labels = labels.data(),
        .state = {
            .mask_len = static_cast<int>(mask.size()),
            .mask = mask.data(),
        },
    };
}

XIDeviceInfo test::CreateXiDeviceInfo(std::array<XIAnyClassInfo*, 3>& info, std::span<char> name) {
    return {
        .deviceid = 1,
        .name = name.data(),
        .use = XIMasterPointer,
        .attachment = 0,
        .enabled = True,
        .num_classes = static_cast<int>(info.size()),
        .classes = info.data()
    };
}

XDeviceInfo test::CreateXDeviceInfo() {
    return {
        .id = 1,
        .type = 1,
        .name = nullptr,
        .num_classes = 0,
        .use = IsXExtensionPointer,
        .inputclassinfo = nullptr
    };
}

XIRawEvent
test::CreateXiRawEvent(int evtype, std::array<unsigned char, 1>& valuator_mask, std::array<double, 1>& values) {
    return {
        .type = GenericEvent,
        .serial = 1,
        .send_event = False,
        .display = nullptr,
        .extension = 0,
        .evtype = evtype,
        .time = 1,
        .deviceid = 1,
        .sourceid = 1,
        .detail = 0,
        .flags = 0,
        .valuators = CreateXiValuatorState(valuator_mask, values),
    };
}

XEvent test::CreateXEvent(XIRawEvent& event) {
    return {
        .xcookie = {
            .type = GenericEvent,
            .serial = 0,
            .send_event = False,
            .display = nullptr,
            .extension = 0,
            .evtype = event.evtype,
            .cookie = 0,
            .data = &event,
        }
    };
}

XIValuatorState
test::CreateXiValuatorState(std::array<unsigned char, 1>& valuator_mask, std::array<double, 1>& values) {
    return {.mask_len = static_cast<int>(valuator_mask.size()), .mask = valuator_mask.data(), .values = values.data()};
}

evgetx11::QueryPointerResult test::CreatePointerResult() {
    return evgetx11::QueryPointerResult{
        .root_x = 1,
        .root_y = 1,
        .button_mask = {nullptr, XFree},
        .modifier_state =
            XIModifierState{
                .base = 0,
                .latched = 0,
                .locked = 0,
                .effective = 0,
            },
        .group_state =
            XIGroupState{
                .base = 0,
                .latched = 0,
                .locked = 0,
                .effective = 0,
            },
        .screen_number = 0
    };
}

void test::SetXWrapperEventMocks(test::X11ApiMock& x_wrapper_mock, XIRawEvent& device_event, XEvent& x_event) {
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
        .WillOnce(testing::Return(testing::ByMove<std::unique_ptr<char[], decltype(&XFree)>>({nullptr, &XFree})));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });
}

void test::SetXWrapperKeyMocks(test::X11ApiMock& x_wrapper_mock, XIRawEvent& device_event, XEvent& x_event) {
    EXPECT_CALL(x_wrapper_mock, EventData)
        .WillOnce(
            testing::Return(testing::ByMove<evgetx11::XEventPointer>({&x_event.xcookie, [](XGenericEventCookie*) {}}))
        );
    EXPECT_CALL(x_wrapper_mock, NextEvent).WillOnce(testing::Return(testing::ByMove(test::CreateXEvent(device_event))));
    EXPECT_CALL(x_wrapper_mock, GetActiveWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, GetFocusWindow)
        .WillOnce(testing::Return(testing::ByMove<std::optional<Window>>({std::nullopt})));
    EXPECT_CALL(x_wrapper_mock, LookupCharacter)
        .WillOnce([](const XIRawEvent&, const evgetx11::QueryPointerResult&, KeySym& key_sym) {
            constexpr std::uint64_t kXkA = 0x0061;
            key_sym = kXkA;
            return "a";
        });
    EXPECT_CALL(x_wrapper_mock, QueryPointer).WillRepeatedly([]() { return test::CreatePointerResult(); });
}

void test::SetXWrapperMocks(test::X11ApiMock& x_wrapper_mock) {
    EXPECT_CALL(x_wrapper_mock, ListInputDevices)
        .WillOnce(
            testing::Return(
                testing::ByMove<std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)>>(
                    {nullptr, [](XDeviceInfo*) {}}
                )
            )
        );
    EXPECT_CALL(x_wrapper_mock, QueryDevice)
        .WillOnce(
            testing::Return(
                testing::ByMove<std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)>>(
                    {nullptr, [](XIDeviceInfo*) {}}
                )
            )
        );
}

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
