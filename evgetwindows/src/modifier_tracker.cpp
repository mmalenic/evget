#include "evgetwindows/modifier_tracker.h"

#include <windows.h>

#include <array>
#include <cstddef>
#include <set>

#include "evget/event/modifier_value.h"
#include "evgetwindows/vk_keysym.h"

void evgetwindows::ModifierTracker::Seed(bool caps_lock, bool num_lock, bool scroll_lock) {
    caps_on_ = caps_lock;
    num_on_ = num_lock;
    scroll_on_ = scroll_lock;
}

void evgetwindows::ModifierTracker::Update(const RAWKEYBOARD& keyboard) {
    const UINT vk = ResolveVk(keyboard);
    const bool is_break = (keyboard.Flags & RI_KEY_BREAK) != 0;

    const bool was_down = vk < down_.size() && down_[vk];
    if (vk < down_.size()) {
        down_[vk] = !is_break;
    }

    // Locks flip on the press transition only, so auto-repeat and release do not double-toggle.
    if (!is_break && !was_down) {
        switch (vk) {
            case VK_CAPITAL:
                caps_on_ = !caps_on_;
                break;
            case VK_NUMLOCK:
                num_on_ = !num_on_;
                break;
            case VK_SCROLL:
                scroll_on_ = !scroll_on_;
                break;
            default:
                break;
        }
    }
}

std::set<evget::ModifierValue> evgetwindows::ModifierTracker::ActiveModifiers() const {
    std::set<evget::ModifierValue> modifiers{};
    if (down_[VK_LSHIFT] || down_[VK_RSHIFT]) {
        modifiers.insert(evget::ModifierValue::kShift);
    }
    if (caps_on_) {
        modifiers.insert(evget::ModifierValue::kCapsLock);
    }
    if (down_[VK_LCONTROL] || down_[VK_RCONTROL]) {
        modifiers.insert(evget::ModifierValue::kControl);
    }
    if (down_[VK_LMENU] || down_[VK_RMENU]) {
        modifiers.insert(evget::ModifierValue::kAlt);
    }
    if (num_on_) {
        modifiers.insert(evget::ModifierValue::kNumLock);
    }
    if (down_[VK_LWIN] || down_[VK_RWIN]) {
        modifiers.insert(evget::ModifierValue::kSuper);
    }
    return modifiers;
}

std::array<BYTE, evgetwindows::kKeyStateSize> evgetwindows::ModifierTracker::KeyState() const {
    std::array<BYTE, kKeyStateSize> state{};
    for (std::size_t vk = 0; vk < state.size(); ++vk) {
        if (down_[vk]) {
            state[vk] = kKeyDownBit;
        }
    }

    // ToUnicodeEx reads the generic modifier VKs.
    if (down_[VK_LSHIFT] || down_[VK_RSHIFT]) {
        state[VK_SHIFT] = kKeyDownBit;
    }
    if (down_[VK_LCONTROL] || down_[VK_RCONTROL]) {
        state[VK_CONTROL] = kKeyDownBit;
    }
    if (down_[VK_LMENU] || down_[VK_RMENU]) {
        state[VK_MENU] = kKeyDownBit;
    }

    if (caps_on_) {
        state[VK_CAPITAL] |= kToggleBit;
    }
    if (num_on_) {
        state[VK_NUMLOCK] |= kToggleBit;
    }
    if (scroll_on_) {
        state[VK_SCROLL] |= kToggleBit;
    }
    return state;
}
