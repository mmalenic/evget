/**
 * \file direction.h
 * \brief Directional enumeration for scrolling.
 */

#ifndef EVGET_EVENT_DIRECTION_H
#define EVGET_EVENT_DIRECTION_H

#include <cstdint>

namespace evget {
/**
 * \brief Represents scrolling directional movement or orientation.
 */
enum class Direction : std::uint8_t {
    kDown, ///< Downward scroll direction
    kLeft, ///< Leftward scroll direction
    kRight, ///< Rightward scroll direction
    kUp ///< Upward scroll direction
};
} // namespace evget

#endif
