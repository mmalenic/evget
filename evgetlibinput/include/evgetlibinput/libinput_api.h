#ifndef EVGETLIBINPUT_LIBINPUT_API_H
#define EVGETLIBINPUT_LIBINPUT_API_H

#include <libinput.h>
#include <libudev.h>

#include <memory>

namespace evgetlibinput {

/**
 * RAII wrapper and API abstraction for libinput
 */
class LibInputApi {
public:
    LibInputApi() = default;
    virtual ~LibInputApi() = default;

    LibInputApi(LibInputApi&&) noexcept = delete;
    LibInputApi& operator=(LibInputApi&&) noexcept = delete;

    LibInputApi(const LibInputApi&) = delete;
    LibInputApi& operator=(const LibInputApi&) = delete;

    // /**
    //  * Initialize libinput context
    //  */
    // evget::Result<void> Initialize();
    //
    // /**
    //  * Get the next event from libinput
    //  */
    // struct libinput_event* GetEvent();
    //
    // /**
    //  * Dispatch pending events
    //  */
    // int Dispatch();
    //
    // /**
    //  * Get the file descriptor for polling
    //  */
    // [[nodiscard]] int GetFd() const;
    //
    // /**
    //  * Check if the context is valid
    //  */
    // [[nodiscard]] bool IsValid() const;
    //
    // /**
    //  * Get the raw libinput context pointer
    //  */
    // [[nodiscard]] struct libinput* Get() const;
};

/**
 * \brief Concrete implementation of the `LibInputApi` interface.
 *
 * This class provides the actual implementation of libinput library function calls,
 * wrapping the low-level libinput API.
 */
class LibInputApiImpl : public LibInputApi {
public:
    LibInputApiImpl();

private:
    std::unique_ptr<udev, decltype(&udev_unref)> udev_context_;
    std::unique_ptr<libinput, decltype(&libinput_unref)> libinput_context_;

    // static int OpenRestricted(const char* path, int flags, void* user_data);
    // static void CloseRestricted(int fd, void* user_data);
    //
    // static const struct libinput_interface interface_;
    //
    // struct libinput* libinput_;
};

} // namespace evgetlibinput

#endif
