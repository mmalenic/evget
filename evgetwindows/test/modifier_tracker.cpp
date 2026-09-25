#include "evgetwindows/modifier_tracker.h"

#include <gtest/gtest.h>

#include <windows.h>

#include <array>

#include "common/windows_mock.h"
#include "evget/event/modifier_value.h"
#include "evgetwindows/raw_event.h"

using test::MakeKeyboard;

TEST(ModifierTracker, InitReflectsInActiveModifiers) {
    evgetwindows::ModifierTracker tracker{};
    tracker.Init(true, false, false);

    const auto modifiers = tracker.Modifiers();
    EXPECT_TRUE(modifiers.contains(evget::ModifierValue::kCapsLock));
    EXPECT_FALSE(modifiers.contains(evget::ModifierValue::kNumLock));
}

TEST(ModifierTracker, MakeAddsAndRemovesModifier) {
    constexpr USHORT kLeftShiftScan = 0x2A;
    evgetwindows::ModifierTracker tracker{};

    tracker.Update(std::get<RAWKEYBOARD>(MakeKeyboard(VK_LSHIFT, kLeftShiftScan, 0).data));
    EXPECT_TRUE(tracker.Modifiers().contains(evget::ModifierValue::kShift));

    tracker.Update(std::get<RAWKEYBOARD>(MakeKeyboard(VK_LSHIFT, kLeftShiftScan, RI_KEY_BREAK).data));
    EXPECT_FALSE(tracker.Modifiers().contains(evget::ModifierValue::kShift));
}

TEST(ModifierTracker, CapsLockChangesOnMake) {
    evgetwindows::ModifierTracker tracker{};

    tracker.Update(std::get<RAWKEYBOARD>(MakeKeyboard(VK_CAPITAL, 0, 0).data));
    EXPECT_TRUE(tracker.Modifiers().contains(evget::ModifierValue::kCapsLock));

    tracker.Update(std::get<RAWKEYBOARD>(MakeKeyboard(VK_CAPITAL, 0, RI_KEY_BREAK).data));
    EXPECT_TRUE(tracker.Modifiers().contains(evget::ModifierValue::kCapsLock));

    tracker.Update(std::get<RAWKEYBOARD>(MakeKeyboard(VK_CAPITAL, 0, 0).data));
    EXPECT_FALSE(tracker.Modifiers().contains(evget::ModifierValue::kCapsLock));
}

TEST(ModifierTracker, ScrollLockDoesNotHaveModifier) {
    evgetwindows::ModifierTracker tracker{};

    tracker.Update(std::get<RAWKEYBOARD>(MakeKeyboard(VK_SCROLL, 0, 0).data));

    const auto modifiers = tracker.Modifiers();
    EXPECT_TRUE(modifiers.empty());
}

TEST(ModifierTracker, KeyStateHasDownOnHeldKey) {
    constexpr USHORT kLeftShiftScan = 0x2A;
    evgetwindows::ModifierTracker tracker{};

    tracker.Update(std::get<RAWKEYBOARD>(MakeKeyboard(VK_LSHIFT, kLeftShiftScan, 0).data));

    const auto state = tracker.KeyState();
    EXPECT_EQ(state[VK_LSHIFT] & evgetwindows::kKeyDownBit, evgetwindows::kKeyDownBit);
    EXPECT_EQ(state[VK_SHIFT] & evgetwindows::kKeyDownBit, evgetwindows::kKeyDownBit);
}
