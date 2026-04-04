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
 * \brief An interface for xkbcommon.
 */
class XkbCommonApi {
public:
    XkbCommonApi() = default;
    virtual ~XkbCommonApi() = default;

    XkbCommonApi(XkbCommonApi&&) noexcept = delete;
    XkbCommonApi& operator=(XkbCommonApi&&) noexcept = delete;

    XkbCommonApi(const XkbCommonApi&) = delete;
    XkbCommonApi& operator=(const XkbCommonApi&) = delete;

    /**
     * \brief Get the xkb keysym name for a key code.
     * \param key the xkb key code
     * \return the keysym name
     */
    virtual std::optional<std::string> GetKeyName(xkb_keycode_t key) = 0;

    /**
     * \brief Get the UTF-8 character for a key code.
     * \param key the xkb key code
     * \return the character, or std::nullopt if unavailable
     */
    virtual std::optional<std::string> GetKeyCharacter(xkb_keycode_t key) = 0;

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
     * \param direction the direction
     */
    virtual void UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) = 0;
};

/**
 * \brief Implementation of the `XkbCommonApi` interface.
 */
class XkbCommon : public XkbCommonApi {
public:
    /**
     * \brief Create a new XkbCommon context.
     * \return a unique pointer to an XkbCommon instance
     */
    static evget::Result<std::unique_ptr<XkbCommon>> New();

    std::optional<std::string> GetKeyName(xkb_keycode_t key) override;

    std::optional<std::string> GetKeyCharacter(xkb_keycode_t key) override;

    bool IsModifierActive(const char* modifier_name) override;

    void UpdateKeyState(xkb_keycode_t key, xkb_key_direction direction) override;

private:
    XkbCommon() = default;

    std::unique_ptr<xkb_context, decltype(&xkb_context_unref)> xkb_context_{nullptr, xkb_context_unref};
    std::unique_ptr<xkb_keymap, decltype(&xkb_keymap_unref)> xkb_key_map_{nullptr, xkb_keymap_unref};
    std::unique_ptr<xkb_state, decltype(&xkb_state_unref)> xkb_state_{nullptr, xkb_state_unref};
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_XKBCOMMON_H
