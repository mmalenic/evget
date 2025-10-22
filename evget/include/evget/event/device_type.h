#ifndef EVGET_EVENT_DEVICE_TYPE_H
#define EVGET_EVENT_DEVICE_TYPE_H

#include <cstdint>

namespace evget {
/**
 * \brief Represents the type of input device that generated an event.
 */
enum class DeviceType : std::uint8_t { 
    kMouse,        ///< Mouse device
    kKeyboard,     ///< Keyboard device
    kTouchpad,     ///< Touchpad device
    kTouchscreen,  ///< Touchscreen device
    kUnknown       ///< Unknown or unrecognized device type
};
} // namespace evget

#endif
