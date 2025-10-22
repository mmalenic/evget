#ifndef EVGET_EVENT_MODIFIER_VALUE_H
#define EVGET_EVENT_MODIFIER_VALUE_H

#include <cstdint>

namespace evget {
/**
 * \brief Specify the allowed keyboard modifiers. Based on https://www.glfw.org/docs/3.3/group__mods.html.
 */
enum class ModifierValue : std::uint8_t {
    kShift,     ///< Shift key modifier
    kCapsLock,  ///< Caps Lock key modifier (X11: Lock)
    kControl,   ///< Control key modifier
    kAlt,       ///< Alt key modifier (X11: Mod1, Mac: Option)
    kNumLock,   ///< Num Lock key modifier (X11: Mod2)
    kMod3,      ///< Generic modifier 3 (X11: Mod3)
    kSuper,     ///< Super key modifier (X11: Mod4, Mac: Command, Windows: Windows key)
    kMod5       ///< Generic modifier 5 (X11: Mod5)
};
} // namespace evget

#endif
