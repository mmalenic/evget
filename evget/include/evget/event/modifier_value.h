#ifndef EVGET_EVENT_MODIFIER_VALUE_H
#define EVGET_EVENT_MODIFIER_VALUE_H

#include <cstdint>

namespace evget {
/**
 * Specify the allowed keyboard modifiers.
 * Based on https://www.glfw.org/docs/3.3/group__mods.html
 */
enum class ModifierValue : std::uint8_t {
    kShift,
    // X11 - Lock
    kCapsLock,
    kControl,
    // X11 - Mod1, Mac - Option
    kAlt,
    // X11 - Mod2
    kNumLock,
    kMod3,
    // X11 - Mod4, Mac - Command, Windows - Windows key
    kSuper,
    kMod5
};
}  // namespace evget

#endif
