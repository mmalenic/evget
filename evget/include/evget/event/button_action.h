/**
 * \file button_action.h
 * \brief Button action enumeration for press, release, and repeat events.
 */

#ifndef EVGET_EVENT_BUTTON_ACTION_H
#define EVGET_EVENT_BUTTON_ACTION_H

#include <cstdint>

namespace evget {
/**
 * \brief Represents the action performed on a button.
 */
enum class ButtonAction : std::uint8_t {
    kPress, ///< Button was pressed down
    kRelease, ///< Button was released
    kRepeat ///< Button is being held down
};
} // namespace evget

#endif
