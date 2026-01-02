#include "evgetlibinput/libinput_api.h"

#include <evget/error.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <unistd.h>

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
