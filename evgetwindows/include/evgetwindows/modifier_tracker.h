/**
 * \file modifier_tracker.h
 * \brief Modifier state tracking.
 */

#ifndef EVGETWINDOWS_MODIFIER_TRACKER_H
#define EVGETWINDOWS_MODIFIER_TRACKER_H

#include <windows.h>

#include <array>
#include <cstddef>
#include <set>

#include "evget/event/modifier_value.h"

namespace evgetwindows {

/**
 * \brief The GetKeyboardState byte flag marking a key as down.
 */
constexpr BYTE kKeyDownBit = 0x80;

/**
 * \brief The GetKeyboardState byte flag marking a lock key as toggled on.
 */
constexpr BYTE kToggleBit = 0x01;

/**
 * \brief The number of key slots in a state array.
 */
constexpr std::size_t kKeyStateSize = 256;

/**
 * \brief Tracks modifiers without querying the Win32 API.
 */
class ModifierTracker {
public:
    ModifierTracker() = default;

    /**
     * \brief Seed the lock toggles once from a startup state.
     * \param caps_lock the initial CapsLock state
     * \param num_lock the initial NumLock state
     * \param scroll_lock the initial ScrollLock state
     */
    void Seed(bool caps_lock, bool num_lock, bool scroll_lock);

    /**
     * \brief Apply a keyboard event.
     * \param keyboard the Raw Input keyboard record
     */
    void Update(const RAWKEYBOARD& keyboard);

    /**
     * \brief The active modifier state.
     * \return the active `ModifierValue`s
     */
    [[nodiscard]] std::set<evget::ModifierValue> ActiveModifiers() const;

    /**
     * \brief The array for the ToUnicodeEx character tracking.
     * \return the character array
     */
    [[nodiscard]] std::array<BYTE, kKeyStateSize> KeyState() const;

private:
    std::array<bool, kKeyStateSize> down_{};
    bool caps_on_{};
    bool num_on_{};
    bool scroll_on_{};
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_MODIFIER_TRACKER_H
