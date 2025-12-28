#include "evgetlibinput/libinput_api.h"

#include <evget/error.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <unistd.h>

#include <cerrno>
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

    // Default udev seat.
    auto seat = libinput_udev_assign_seat(lib_input->libinput_context_.get(), "seat0");
    if (seat != 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to assign udev seat"}
        };
    }

    return std::move(lib_input);
}

evget::Result<evgetlibinput::InputEvent> evgetlibinput::LibInputApiImpl::GetEvent() {
    auto dispatch = libinput_dispatch(libinput_context_.get());
    if (dispatch != 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to dispatch next event"}
        };
    }

    auto* event = libinput_get_event(libinput_context_.get());
    return evget::Result<InputEvent>{{event, libinput_event_destroy}};
}

libinput_event_type evgetlibinput::LibInputApiImpl::GetEventType(libinput_event& event) {
    return libinput_event_get_type(&event);
}

libinput_event_pointer* evgetlibinput::LibInputApiImpl::GetPointerEvent(libinput_event& event) {
    return libinput_event_get_pointer_event(&event);
}
