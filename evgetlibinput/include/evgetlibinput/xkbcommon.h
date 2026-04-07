/**
 * \file xkbcommon.h
 * \brief Abstraction layer for xkbcommon.
 */

#ifndef EVGETLIBINPUT_XKBCOMMON_H
#define EVGETLIBINPUT_XKBCOMMON_H

#include <xkbcommon/xkbcommon.h>

#include <memory>
#include <optional>
#include <string>

#include "evget/error.h"

namespace evgetlibinput {

/**
 * \brief A wrapper around xkbcommon that tracks keymap and modifier state.
 */
class XkbCommon {
public:
    /**
     * \brief Create a new XkbCommon context using system/environment defaults.
     * \return a unique pointer to an XkbCommon instance
     */
    static evget::Result<XkbCommon> New();

    /**
     * \brief Create a new XkbCommon context with an explicit keymap layout.
     * \param names xkb rule names
     * \return a unique pointer to an XkbCommon instance
     */
    static evget::Result<XkbCommon> NewWithNames(const xkb_rule_names* names);

    /**
     * \brief Get the xkb keysym name for a key code.
     * \param key the xkb key code
     * \return the keysym name
     */
    [[nodiscard]] std::optional<std::string> GetKeyName(xkb_keycode_t key) const;

    /**
     * \brief Get the UTF-8 character for a key code.
     * \param key the xkb key code
     * \return the character, or std::nullopt if unavailable
     */
    [[nodiscard]] std::optional<std::string> GetKeyCharacter(xkb_keycode_t key) const;

    /**
     * \brief Check if the effective xkb modifier with the given name is active in the xkb state. For modifiers to be
     *        active, previous calls to `UpdateKeyState` should happen in response to libinput key events.
     * \param modifier_name modifier name
     * \return if the modifier is active
     */
    bool IsModifierActive(const char* modifier_name) const;

    /**
     * \brief Update the key state for a given key.
     * \param key the key to update
     * \param direction the direction
     */
    void UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) const;

private:
    XkbCommon() = default;

    std::unique_ptr<xkb_context, decltype(&xkb_context_unref)> xkb_context_{nullptr, xkb_context_unref};
    std::unique_ptr<xkb_keymap, decltype(&xkb_keymap_unref)> xkb_key_map_{nullptr, xkb_keymap_unref};
    std::unique_ptr<xkb_state, decltype(&xkb_state_unref)> xkb_state_{nullptr, xkb_state_unref};
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_XKBCOMMON_H
