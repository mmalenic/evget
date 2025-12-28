/**
 * \file libinput_api.h
 * \brief The libinput API abstraction layer for all libinput functions used within evget.
 */

#ifndef EVGETLIBINPUT_LIBINPUT_API_H
#define EVGETLIBINPUT_LIBINPUT_API_H

#include <libinput.h>
#include <libudev.h>

#include <memory>

#include "evget/error.h"

namespace evgetlibinput {
/**
 * \brief A pointer to an input event with a libinput deleter.
 */
using InputEvent = std::unique_ptr<libinput_event, decltype(&libinput_event_destroy)>;

/**
 * \brief API abstraction for libinput.
 */
class LibInputApi {
public:
    LibInputApi() = default;
    virtual ~LibInputApi() = default;

    LibInputApi(LibInputApi&&) noexcept = delete;
    LibInputApi& operator=(LibInputApi&&) noexcept = delete;

    LibInputApi(const LibInputApi&) = delete;
    LibInputApi& operator=(const LibInputApi&) = delete;

    /**
     * \brief Get the next event from libinput.
     * \return the next event from the queue
     */
    virtual evget::Result<InputEvent> GetEvent() = 0;

    /**
     * \brief Get the type of libinput event.
     * \param event libinput event
     * @return the event type
     */
    virtual libinput_event_type GetEventType(libinput_event& event) = 0;

    /**
     * \brief Get a pointer event from the libinput event. The type must be checked
     *        before calling this function.
     * @param event libinput event
     * @return pointer event
     */
    virtual libinput_event_pointer* GetPointerEvent(libinput_event& event) = 0;
};

/**
 * \brief Concrete implementation of the `LibInputApi` interface.
 *
 * This class provides the actual implementation of libinput library function calls,
 * wrapping the low-level libinput API.
 */
class LibInputApiImpl : public LibInputApi {
public:
    /**
     * \brief Create a new LibInputApi context as a pointer to the interface.
     * \return a unique pointer to a LibInputApi context
     */
    static evget::Result<std::unique_ptr<LibInputApi>> New();

    evget::Result<std::unique_ptr<libinput_event, decltype(&libinput_event_destroy)>> GetEvent() override;

    libinput_event_type GetEventType(libinput_event& event) override;

    libinput_event_pointer* GetPointerEvent(libinput_event& event) override;

private:
    LibInputApiImpl() = default;

    std::unique_ptr<udev, decltype(&udev_unref)> udev_context_{nullptr, udev_unref};
    std::unique_ptr<libinput, decltype(&libinput_unref)> libinput_context_{nullptr, libinput_unref};
};

} // namespace evgetlibinput

#endif
