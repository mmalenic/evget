#ifndef EVGET_EVENT_DIRECTION_H
#define EVGET_EVENT_DIRECTION_H

#include <cstdint>

namespace evget {
enum class Direction : std::uint8_t { kDown, kLeft, kRight, kUp };
}  // namespace evget

#endif
