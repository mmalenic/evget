/**
 * \file vk_keysym.h
 * \brief Virtual-key to X11 keysym mapping.
 */

#ifndef EVGETWINDOWS_VK_KEYSYM_H
#define EVGETWINDOWS_VK_KEYSYM_H

#include <windows.h>

#include <string>

namespace evgetwindows {

/**
 * \brief Scan code that VK_RSHIFT maps to.
 * \return the scan code
 */
[[nodiscard]] USHORT RightShiftScanCode();

/**
 * \brief The name or modifier key mapping to its X11 name.
 * \param key the virtual key code
 * \param extended_key_bit the RI_KEY_E0 bit
 * \return the keysymdef name or a default value
 */
[[nodiscard]] std::string NamedKeysym(UINT key, bool extended_key_bit);

/**
 * \brief The virtual key mapped to the X11 keysymdef name.
 * \param key the virtual key code
 * \param extended_key_bit the RI_KEY_E0 bit
 * \return the keysymdef name or a default value
 */
[[nodiscard]] std::string VkToKeysymName(UINT key, bool extended_key_bit);

/**
 * \brief Resolves a modifier key to the left or right variant.
 * \param keyboard the Raw Input keyboard
 * \return the resolved key
 */
[[nodiscard]] UINT ResolveVk(const RAWKEYBOARD& keyboard);

} // namespace evgetwindows

#endif // EVGETWINDOWS_VK_KEYSYM_H
