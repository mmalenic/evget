/**
 * \file evdev.h
 * \brief Abstraction layer for libevdev.
 */

#ifndef EVGETLIBINPUT_EVDEV_H
#define EVGETLIBINPUT_EVDEV_H

#include <cstdint>

namespace evgetlibinput {

/**
 * \brief An interface for libevdev.
 */
class EvdevApi {
public:
    EvdevApi() = default;
    virtual ~EvdevApi() = default;

    EvdevApi(EvdevApi&&) noexcept = delete;
    EvdevApi& operator=(EvdevApi&&) noexcept = delete;

    EvdevApi(const EvdevApi&) = delete;
    EvdevApi& operator=(const EvdevApi&) = delete;

    /**
     * \brief Get the name of an event code for a given event type.
     * \param type the event type
     * \param code the event code
     * \return the event code name or nullptr if invalid
     */
    virtual const char* EventCodeName(std::uint32_t type, std::uint32_t code) = 0;
};

/**
 * \brief Concrete implementation of the `EvdevApi` interface using libevdev.
 */
class Evdev : public EvdevApi {
public:
    const char* EventCodeName(std::uint32_t type, std::uint32_t code) override;
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_EVDEV_H
