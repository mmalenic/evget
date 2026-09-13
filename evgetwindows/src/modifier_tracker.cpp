#include "evgetwindows/modifier_tracker.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <set>

#include "evget/event/modifier_value.h"
#include "evgetwindows/vk_keysym.h"

void evgetwindows::ModifierTracker::Init(bool caps_lock, bool num_lock, bool scroll_lock) {
    caps_on_ = caps_lock;
    num_on_ = num_lock;
    scroll_on_ = scroll_lock;
}

void evgetwindows::ModifierTracker::Update(const RAWKEYBOARD& keyboard) {
    const UINT key = ResolveVk(keyboard);
    if (key >= down_.size()) {
        return;
    }
    const bool is_break = (keyboard.Flags & RI_KEY_BREAK) != 0;

    const bool was_down = down_.at(key);
    down_.at(key) = !is_break;

    // Locks go on the press only and auto-repeat/release should not double the toggle.
    if (!is_break && !was_down) {
        switch (key) {
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

std::set<evget::ModifierValue> evgetwindows::ModifierTracker::Modifiers() const {
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
    std::ranges::transform(down_, state.begin(), [](const bool is_down) { return is_down ? kKeyDownBit : BYTE{0}; });

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
