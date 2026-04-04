#include "evgetlibinput/libinput.h"

#include <evget/error.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include <array>
#include <cerrno>
#include <cstdint>
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

evget::Result<std::unique_ptr<evgetlibinput::LibInput>> evgetlibinput::LibInput::New() {
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
    lib_input->pollfd_.events = POLLIN;
    lib_input->pollfd_.revents = 0;
    lib_input->wait_for_poll_ = true;

    // Default udev seat.
    auto seat = libinput_udev_assign_seat(lib_input->libinput_context_.get(), "seat0");
    if (seat != 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to assign udev seat"}
        };
    }

    // Initialize xkb context and state.
    lib_input->xkb_context_ = {xkb_context_new(XKB_CONTEXT_NO_FLAGS), xkb_context_unref};
    if (lib_input->xkb_context_ == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize xkb context"}
        };
    }
    lib_input->xkb_key_map_ = {
        // Recommended to use XKB_KEYMAP_FORMAT_TEXT_V1 for now instead of V2:
        // https://xkbcommon.org/doc/current/group__keymap.html#gab0f75d6cc5773e5dd404e2c3f61366a3
        xkb_keymap_new_from_names2(
            lib_input->xkb_context_.get(),
            nullptr,
            XKB_KEYMAP_FORMAT_TEXT_V1,
            XKB_KEYMAP_COMPILE_NO_FLAGS
        ),
        xkb_keymap_unref
    };
    if (lib_input->xkb_key_map_ == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize xkb keymap"}
        };
    }
    lib_input->xkb_state_ = {xkb_state_new(lib_input->xkb_key_map_.get()), xkb_state_unref};
    if (lib_input->xkb_state_ == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to initialize xkb state"}
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
            if (poll(&pollfd_, 1, -1) <= 0) {
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

bool evgetlibinput::LibInput::IsModifierActive(const char* modifier_name) {
    return xkb_state_mod_name_is_active(this->xkb_state_.get(), modifier_name, XKB_STATE_MODS_EFFECTIVE) != 0;
}

void evgetlibinput::LibInput::UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) {
    // From xkb's perspective, evget is considered a server as it handles libinput events directly, so
    // use the server version of the updating state rather than xkb_state_update_mask.
    xkb_state_update_key(this->xkb_state_.get(), key, direction);
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

std::optional<std::string> evgetlibinput::LibInput::GetKeyName(xkb_keycode_t key) {
    auto sym = xkb_state_key_get_one_sym(xkb_state_.get(), key);
    if (sym == XKB_KEY_NoSymbol) {
        return std::nullopt;
    }

    // xkb_keysym_get_name recommends a buffer of 64 bytes.
    constexpr auto kKeySymBufSize = 64;
    std::array<char, kKeySymBufSize> buf{};
    auto needed = xkb_keysym_get_name(sym, buf.data(), buf.size());
    if (needed <= 0) {
        return std::nullopt;
    }

    if (needed < buf.size()) {
        // String output is null-terminated.
        return std::string{buf.data()};
    }

    // If this has failed for some reason, try again with the correct amount of bytes.
    std::string name(needed, '\0');
    xkb_keysym_get_name(sym, name.data(), name.size() + 1);
    return name;
}

std::optional<std::string> evgetlibinput::LibInput::GetKeyCharacter(xkb_keycode_t key) {
    // xkb_state_key_get_utf8 with null buffer returns the required size.
    auto size = xkb_state_key_get_utf8(xkb_state_.get(), key, nullptr, 0);
    if (size <= 0) {
        return std::nullopt;
    }

    std::string character(size, '\0');
    xkb_state_key_get_utf8(xkb_state_.get(), key, character.data(), character.size() + 1);
    return character;
}
