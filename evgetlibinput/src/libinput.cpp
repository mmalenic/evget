#include "evgetlibinput/libinput.h"

#include <evget/error.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace {
constexpr int OpenRestricted(const char* path, int flags, void* /* unused */) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    const int file = open(path, flags);
    return file < 0 ? -errno : file;
}

constexpr void CloseRestricted(int file, void* /* unused */) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    close(file);
}
} // namespace

constexpr libinput_interface kLibInputInterface = {
    .open_restricted = OpenRestricted,
    .close_restricted = CloseRestricted,
};

evget::Result<std::unique_ptr<evgetlibinput::LibInput>> evgetlibinput::LibInput::New(
    const std::optional<std::string>& seat
) {
    auto lib_input = std::unique_ptr<LibInput>(new LibInput{});

    lib_input->udev_context_ = {udev_new(), udev_unref};
    if (lib_input->udev_context_ == nullptr) {
        return evget::Err{{.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize udev"}};
    }

    lib_input->libinput_context_ = {
        libinput_udev_create_context(&kLibInputInterface, nullptr, lib_input->udev_context_.get()),
        libinput_unref
    };
    if (lib_input->libinput_context_ == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize libinput"}
        };
    }

    lib_input->pollfd_.fd = libinput_get_fd(lib_input->libinput_context_.get());
    lib_input->pollfd_.events = POLLIN; // NOLINT(misc-include-cleaner)
    lib_input->pollfd_.revents = 0;
    lib_input->wait_for_poll_ = true;

    const std::string& seat_name = seat.value_or("seat0");
    auto assign = libinput_udev_assign_seat(lib_input->libinput_context_.get(), seat_name.c_str());
    if (assign != 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError,
             .message = std::format("unable to assign udev seat '{}'", seat_name)}
        };
    }

    return std::move(lib_input);
}

evget::Result<evgetlibinput::LibInputEvent> evgetlibinput::LibInput::GetEvent() {
    libinput_event* event = nullptr;
    // Loop until there is a non-null event, polling and dispatching the context if the current
    // event is null.
    while (event == nullptr) {
        if (wait_for_poll_) {
            int poll_result = poll(&pollfd_, 1, -1); // NOLINT(misc-include-cleaner)
            while (poll_result < 0 && errno == EINTR) {
                poll_result = poll(&pollfd_, 1, -1); // NOLINT(misc-include-cleaner)
            }

            if (poll_result <= 0) {
                return evget::Err{
                    {.error_type = evget::ErrorType::kEventHandlerError, .message = "polling for events failed"}
                };
            }

            auto dispatch = libinput_dispatch(libinput_context_.get());
            if (dispatch != 0) {
                return evget::Err{
                    {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to dispatch next event"}
                };
            }
        }

        event = libinput_get_event(libinput_context_.get());
        wait_for_poll_ = event == nullptr;
    }

    return evget::Result<LibInputEvent>{{event, libinput_event_destroy}};
}

libinput_event_type evgetlibinput::LibInput::GetEventType(libinput_event& event) {
    return libinput_event_get_type(&event);
}

libinput_event_pointer* evgetlibinput::LibInput::GetPointerEvent(libinput_event& event) {
    return libinput_event_get_pointer_event(&event);
}

libinput_device* evgetlibinput::LibInput::GetDevice(libinput_event& event) {
    return libinput_event_get_device(&event);
}

std::uint64_t evgetlibinput::LibInput::GetPointerTimeMicroseconds(libinput_event_pointer& event) {
    return libinput_event_pointer_get_time_usec(&event);
}

int evgetlibinput::LibInput::GetDeviceFingerCount(libinput_device& device) {
    return libinput_device_config_tap_get_finger_count(&device);
}

bool evgetlibinput::LibInput::DeviceHasCapability(libinput_device& device, libinput_device_capability capability) {
    return libinput_device_has_capability(&device, capability) != 0;
}

double evgetlibinput::LibInput::GetPointerDx(libinput_event_pointer& event) {
    return libinput_event_pointer_get_dx(&event);
}

double evgetlibinput::LibInput::GetPointerDy(libinput_event_pointer& event) {
    return libinput_event_pointer_get_dy(&event);
}

const char* evgetlibinput::LibInput::GetDeviceName(libinput_device& device) {
    return libinput_device_get_name(&device);
}

double evgetlibinput::LibInput::GetPointerAbsoluteX(libinput_event_pointer& event, std::uint32_t width) {
    return libinput_event_pointer_get_absolute_x_transformed(&event, width);
}

double evgetlibinput::LibInput::GetPointerAbsoluteY(libinput_event_pointer& event, std::uint32_t width) {
    return libinput_event_pointer_get_absolute_y_transformed(&event, width);
}

std::uint32_t evgetlibinput::LibInput::GetPointerButton(libinput_event_pointer& event) {
    return libinput_event_pointer_get_button(&event);
}

libinput_button_state evgetlibinput::LibInput::GetPointerButtonState(libinput_event_pointer& event) {
    return libinput_event_pointer_get_button_state(&event);
}

bool evgetlibinput::LibInput::GetPointerHasAxis(libinput_event_pointer& event, libinput_pointer_axis axis) {
    return libinput_event_pointer_has_axis(&event, axis) != 0;
}

double evgetlibinput::LibInput::GetPointerScrollValue(libinput_event_pointer& event, libinput_pointer_axis axis) {
    return libinput_event_pointer_get_scroll_value(&event, axis);
}

libinput_event_tablet_tool* evgetlibinput::LibInput::GetTabletToolEvent(libinput_event& event) {
    return libinput_event_get_tablet_tool_event(&event);
}

std::uint64_t evgetlibinput::LibInput::GetTabletToolTimeMicroseconds(libinput_event_tablet_tool& event) {
    return libinput_event_tablet_tool_get_time_usec(&event);
}

double evgetlibinput::LibInput::GetTabletToolDx(libinput_event_tablet_tool& event) {
    return libinput_event_tablet_tool_get_dx(&event);
}

double evgetlibinput::LibInput::GetTabletToolDy(libinput_event_tablet_tool& event) {
    return libinput_event_tablet_tool_get_dy(&event);
}

std::uint32_t evgetlibinput::LibInput::GetTabletToolButton(libinput_event_tablet_tool& event) {
    return libinput_event_tablet_tool_get_button(&event);
}

libinput_button_state evgetlibinput::LibInput::GetTabletToolButtonState(libinput_event_tablet_tool& event) {
    return libinput_event_tablet_tool_get_button_state(&event);
}

libinput_tablet_tool_tip_state evgetlibinput::LibInput::GetTabletToolTipState(libinput_event_tablet_tool& event) {
    return libinput_event_tablet_tool_get_tip_state(&event);
}

libinput_tablet_tool_proximity_state evgetlibinput::LibInput::GetTabletToolProximityState(
    libinput_event_tablet_tool& event
) {
    return libinput_event_tablet_tool_get_proximity_state(&event);
}

libinput_event_tablet_pad* evgetlibinput::LibInput::GetTabletPadEvent(libinput_event& event) {
    return libinput_event_get_tablet_pad_event(&event);
}

std::uint64_t evgetlibinput::LibInput::GetTabletPadTimeMicroseconds(libinput_event_tablet_pad& event) {
    return libinput_event_tablet_pad_get_time_usec(&event);
}

std::uint32_t evgetlibinput::LibInput::GetTabletPadButtonNumber(libinput_event_tablet_pad& event) {
    return libinput_event_tablet_pad_get_button_number(&event);
}

libinput_button_state evgetlibinput::LibInput::GetTabletPadButtonState(libinput_event_tablet_pad& event) {
    return libinput_event_tablet_pad_get_button_state(&event);
}

std::uint32_t evgetlibinput::LibInput::GetTabletPadKey(libinput_event_tablet_pad& event) {
    return libinput_event_tablet_pad_get_key(&event);
}

libinput_key_state evgetlibinput::LibInput::GetTabletPadKeyState(libinput_event_tablet_pad& event) {
    return libinput_event_tablet_pad_get_key_state(&event);
}

libinput_event_touch* evgetlibinput::LibInput::GetTouchEvent(libinput_event& event) {
    return libinput_event_get_touch_event(&event);
}

std::uint64_t evgetlibinput::LibInput::GetTouchTimeMicroseconds(libinput_event_touch& event) {
    return libinput_event_touch_get_time_usec(&event);
}

double evgetlibinput::LibInput::GetTouchX(libinput_event_touch& event, std::uint32_t width) {
    return libinput_event_touch_get_x_transformed(&event, width);
}

double evgetlibinput::LibInput::GetTouchY(libinput_event_touch& event, std::uint32_t height) {
    return libinput_event_touch_get_y_transformed(&event, height);
}

std::int32_t evgetlibinput::LibInput::GetTouchSeatSlot(libinput_event_touch& event) {
    return libinput_event_touch_get_seat_slot(&event);
}

libinput_event_keyboard* evgetlibinput::LibInput::GetKeyboardEvent(libinput_event& event) {
    return libinput_event_get_keyboard_event(&event);
}

std::uint64_t evgetlibinput::LibInput::GetKeyboardTimeMicroseconds(libinput_event_keyboard& event) {
    return libinput_event_keyboard_get_time_usec(&event);
}

std::uint32_t evgetlibinput::LibInput::GetKeyboardKey(libinput_event_keyboard& event) {
    return libinput_event_keyboard_get_key(&event);
}

libinput_key_state evgetlibinput::LibInput::GetKeyboardKeyState(libinput_event_keyboard& event) {
    return libinput_event_keyboard_get_key_state(&event);
}
