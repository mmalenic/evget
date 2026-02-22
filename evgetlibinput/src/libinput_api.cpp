#include "evgetlibinput/libinput_api.h"

#include <evget/error.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include <cerrno>
#include <cstdint>
#include <memory>
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

evget::Result<std::unique_ptr<evgetlibinput::LibInputApi>> evgetlibinput::LibInputApiImpl::New() {
    auto lib_input = std::unique_ptr<LibInputApiImpl>(new LibInputApiImpl{});

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

evget::Result<evgetlibinput::LibInputEvent> evgetlibinput::LibInputApiImpl::GetEvent() {
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

libinput_event_type evgetlibinput::LibInputApiImpl::GetEventType(libinput_event& event) {
    return libinput_event_get_type(&event);
}

libinput_event_pointer* evgetlibinput::LibInputApiImpl::GetPointerEvent(libinput_event& event) {
    return libinput_event_get_pointer_event(&event);
}

libinput_device* evgetlibinput::LibInputApiImpl::GetDevice(libinput_event& event) {
    return libinput_event_get_device(&event);
}

std::uint64_t evgetlibinput::LibInputApiImpl::GetPointerTimeMicroseconds(libinput_event_pointer& event) {
    return libinput_event_pointer_get_time_usec(&event);
}

int evgetlibinput::LibInputApiImpl::GetDeviceFingerCount(libinput_device& device) {
    return libinput_device_config_tap_get_finger_count(&device);
}

bool evgetlibinput::LibInputApiImpl::DeviceHasCapability(
    libinput_device& device,
    libinput_device_capability capability
) {
    return libinput_device_has_capability(&device, capability) != 0;
}

double evgetlibinput::LibInputApiImpl::GetPointerDx(libinput_event_pointer& event) {
    return libinput_event_pointer_get_dx(&event);
}

double evgetlibinput::LibInputApiImpl::GetPointerDy(libinput_event_pointer& event) {
    return libinput_event_pointer_get_dy(&event);
}

const char* evgetlibinput::LibInputApiImpl::GetDeviceName(libinput_device& device) {
    return libinput_device_get_name(&device);
}

bool evgetlibinput::LibInputApiImpl::IsModifierActive(const char* modifier_name) {
    return xkb_state_mod_name_is_active(this->xkb_state_.get(), modifier_name, XKB_STATE_MODS_EFFECTIVE) != 0;
}

void evgetlibinput::LibInputApiImpl::UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) {
    // From xkb's perspective, evget is considered a server as it handles libinput events directly, so
    // use the server version of the updating state rather than xkb_state_update_mask.
    xkb_state_update_key(this->xkb_state_.get(), key, direction);
}

double evgetlibinput::LibInputApiImpl::GetPointerAbsoluteX(libinput_event_pointer& event) {
    return libinput_event_pointer_get_absolute_x(&event);
}

double evgetlibinput::LibInputApiImpl::GetPointerAbsoluteY(libinput_event_pointer& event) {
    return libinput_event_pointer_get_absolute_y(&event);
}
