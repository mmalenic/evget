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
    auto lib_input = std::make_unique<LibInputApiImpl>();

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

    auto seat = libinput_udev_assign_seat(lib_input->libinput_context_.get(), "evget");
    if (seat != 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to assign udev seat"}
        };
    }

    return std::move(lib_input);
}

evget::Result<std::unique_ptr<libinput_event, decltype(&libinput_event_destroy)>>
evgetlibinput::LibInputApiImpl::GetEvent() {
    auto dispatch = libinput_dispatch(libinput_context_.get());
    if (dispatch != 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to dispatch next event"}
        };
    }

    auto* event = libinput_get_event(libinput_context_.get());
    return evget::Result<std::unique_ptr<libinput_event, decltype(&libinput_event_destroy)>>{
        {event, libinput_event_destroy}
    };
}

// namespace evgetlibinput {
//
// const struct libinput_interface LibinputApi::interface_ = {
//     .open_restricted = LibinputApi::OpenRestricted,
//     .close_restricted = LibinputApi::CloseRestricted,
// };
//
// LibinputApi::LibinputApi(UdevContext& udev_context) : udev_context_(udev_context), libinput_(nullptr) {}
//
// LibinputApi::~LibinputApi() {
//     if (libinput_) {
//         libinput_unref(libinput_);
//     }
// }
//
// LibinputApi::LibinputApi(LibinputApi&& other) noexcept
//     : udev_context_(other.udev_context_), libinput_(other.libinput_) {
//     other.libinput_ = nullptr;
// }
//
// LibinputApi& LibinputApi::operator=(LibinputApi&& other) noexcept {
//     if (this != &other) {
//         if (libinput_) {
//             libinput_unref(libinput_);
//         }
//         libinput_ = other.libinput_;
//         other.libinput_ = nullptr;
//     }
//     return *this;
// }
//
// evget::Result<void> LibinputApi::Initialize() {
//     if (!udev_context_.IsValid()) {
//         return evget::Error("Udev context is not valid");
//     }
//
//     libinput_ = libinput_udev_create_context(&interface_, this, udev_context_.Get());
//     if (!libinput_) {
//         return evget::Error("Failed to create libinput context");
//     }
//
//     // Assign seat - typically "seat0" for the default seat
//     if (libinput_udev_assign_seat(libinput_, "seat0") != 0) {
//         libinput_unref(libinput_);
//         libinput_ = nullptr;
//         return evget::Error("Failed to assign seat to libinput");
//     }
//
//     spdlog::info("Libinput context initialized successfully");
//     return {};
// }
//
// struct libinput_event* LibinputApi::GetEvent() {
//     if (!libinput_) {
//         return nullptr;
//     }
//     return libinput_get_event(libinput_);
// }
//
// int LibinputApi::Dispatch() {
//     if (!libinput_) {
//         return -1;
//     }
//     return libinput_dispatch(libinput_);
// }
//
// int LibinputApi::GetFd() const {
//     if (!libinput_) {
//         return -1;
//     }
//     return libinput_get_fd(libinput_);
// }
//
// bool LibinputApi::IsValid() const {
//     return libinput_ != nullptr;
// }
//
// struct libinput* LibinputApi::Get() const {
//     return libinput_;
// }
//
// int LibinputApi::OpenRestricted(const char* path, int flags, void* user_data) {
//     int fd = open(path, flags);
//     if (fd < 0) {
//         spdlog::warn("Failed to open device: {}", path);
//     } else {
//         spdlog::debug("Opened device: {} (fd: {})", path, fd);
//     }
//     return fd;
// }
//
// void LibinputApi::CloseRestricted(int fd, void* user_data) {
//     spdlog::debug("Closing device fd: {}", fd);
//     close(fd);
// }

// } // namespace evgetlibinput
