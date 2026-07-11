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
 * \brief Map a virtual-key to its X11 keysymdef name.
 * \param vk the virtual-key code
 * \param e0 the RI_KEY_E0 extended-key bit
 * \return the keysymdef name, or a non-empty fallback for an unmapped key
 */
[[nodiscard]] std::string VkToKeysymName(UINT vk, bool e0);

/**
 * \brief Resolve a generic modifier virtual-key to its left or right variant.
 * \param keyboard the Raw Input keyboard record
 * \return the virtual-key, or VKey unchanged when already specific
 */
[[nodiscard]] UINT ResolveVk(const RAWKEYBOARD& keyboard);

} // namespace evgetwindows

#endif // EVGETWINDOWS_VK_KEYSYM_H
