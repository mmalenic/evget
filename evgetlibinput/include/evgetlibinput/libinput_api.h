/**
 * \file libinput_api.h
 * \brief The libinput API abstraction layer for all libinput functions used within evget.
 */

#ifndef EVGETLIBINPUT_LIBINPUT_API_H
#define EVGETLIBINPUT_LIBINPUT_API_H

#include <libinput.h>
#include <libudev.h>
#include <sys/poll.h>
#include <xkbcommon/xkbcommon.h>

#include <memory>

#include "evget/error.h"

namespace evgetlibinput {
/**
 * \brief A pointer to an input event with a libinput deleter.
 */
using LibInputEvent = std::unique_ptr<libinput_event, decltype(&libinput_event_destroy)>;

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
    virtual evget::Result<LibInputEvent> GetEvent() = 0;

    /**
     * \brief Get the type of libinput event.
     * \param event libinput event
     * \return the event type
     */
    virtual libinput_event_type GetEventType(libinput_event& event) = 0;

    /**
     * \brief Get a pointer event from the libinput event. The type must be checked
     *        before calling this function.
     * \param event libinput event
     * \return pointer event
     */
    virtual libinput_event_pointer* GetPointerEvent(libinput_event& event) = 0;

    /**
     * \brief Get the event time for a pointer event.
     * \param event pointer event
     * \return event time in microseconds
     */
    virtual std::uint64_t GetPointerTimeMicroseconds(libinput_event_pointer& event) = 0;

    /**
     * \brief Get the change in x position from the previous event.
     * \param event event
     * \return change in x direction
     */
    virtual double GetPointerDx(libinput_event_pointer& event) = 0;

    /**
     * \brief Get the change in y position from the previous event.
     * \param event event
     * \return change in y direction
     */
    virtual double GetPointerDy(libinput_event_pointer& event) = 0;

    /**
     * \brief Get the x position for absolute pointer event.
     * \param event event
     * \return x position
     */
    virtual double GetPointerAbsoluteX(libinput_event_pointer& event) = 0;

    /**
     * \brief Get the y position for absolute pointer event.
     * \param event event
     * \return y position
     */
    virtual double GetPointerAbsoluteY(libinput_event_pointer& event) = 0;

    /**
     * \brief Get the device associated with the event.
     * \param event the associated event
     * \return the libinput device
     */
    virtual libinput_device* GetDevice(libinput_event& event) = 0;

    /**
     * \brief Get the number of fingers that can be used for tap to click if supported.
     * \param device device to check
     * \return number of fingers
     */
    virtual int GetDeviceFingerCount(libinput_device& device) = 0;

    /**
     * \brief Check if a device has the capability.
     * \param device device to check
     * \param capability capability
     * \return true or false indicating if the device has the capability
     */
    virtual bool DeviceHasCapability(libinput_device& device, libinput_device_capability capability) = 0;

    /**
     * \brief Get the name of the device.
     * \param device device to get name of
     * \return name of device
     */
    virtual const char* GetDeviceName(libinput_device& device) = 0;

    /**
     * \brief Check if the effective xkb modifier with the given name is active in the xkb state. For modifiers to be
     *        active, previous calls to `UpdateKeyState` should happen in response to libinput key events.
     * \param modifier_name modifier name
     * \return if the modifier is active
     */
    virtual bool IsModifierActive(const char* modifier_name) = 0;

    /**
     * \brief Update the key state for a given key.
     * \param key the key to update
     * \param direction the direction, either pressed or released
     */
    virtual void UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) = 0;
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

    libinput_device* GetDevice(libinput_event& event) override;

    std::uint64_t GetPointerTimeMicroseconds(libinput_event_pointer& event) override;

    int GetDeviceFingerCount(libinput_device& device) override;

    bool DeviceHasCapability(libinput_device& device, libinput_device_capability capability) override;

    double GetPointerDx(libinput_event_pointer& event) override;

    double GetPointerDy(libinput_event_pointer& event) override;

    const char* GetDeviceName(libinput_device& device) override;

    bool IsModifierActive(const char* modifier_name) override;

    void UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) override;

    double GetPointerAbsoluteX(libinput_event_pointer& event) override;

    double GetPointerAbsoluteY(libinput_event_pointer& event) override;

private:
    LibInputApiImpl() = default;

    std::unique_ptr<udev, decltype(&udev_unref)> udev_context_{nullptr, udev_unref};
    std::unique_ptr<libinput, decltype(&libinput_unref)> libinput_context_{nullptr, libinput_unref};

    std::unique_ptr<xkb_context, decltype(&xkb_context_unref)> xkb_context_{nullptr, xkb_context_unref};
    std::unique_ptr<xkb_keymap, decltype(&xkb_keymap_unref)> xkb_key_map_{nullptr, xkb_keymap_unref};
    std::unique_ptr<xkb_state, decltype(&xkb_state_unref)> xkb_state_{nullptr, xkb_state_unref};

    pollfd pollfd_{};
    bool wait_for_poll_{};
};

} // namespace evgetlibinput

#endif
