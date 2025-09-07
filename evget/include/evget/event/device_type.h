#ifndef EVGET_EVENT_DEVICE_TYPE_H
#define EVGET_EVENT_DEVICE_TYPE_H

#include <cstdint>

namespace evget {
enum class DeviceType : std::uint8_t { kMouse, kKeyboard, kTouchpad, kTouchscreen, kUnknown };
} // namespace evget

#endif
