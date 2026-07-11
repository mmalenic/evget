#include "evgetwindows/vk_keysym.h"

#include <gtest/gtest.h>

#include <windows.h>

#include <set>
#include <string>
#include <variant>
#include <vector>

#include "common/windows_mock.h"
#include "evgetwindows/raw_event.h"

using test::MakeKeyboard;

TEST(VkKeysym, NamedKeysHaveExactSpellings) {
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_RETURN, false), "Return");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_SPACE, false), "space");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_ESCAPE, false), "Escape");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_TAB, false), "Tab");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_BACK, false), "BackSpace");
}

TEST(VkKeysym, ModifierKeysAreHandednessSpecific) {
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_LSHIFT, false), "Shift_L");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_RSHIFT, false), "Shift_R");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_LCONTROL, false), "Control_L");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_RCONTROL, false), "Control_R");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_LMENU, false), "Alt_L");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_RMENU, false), "Alt_R");
}

TEST(VkKeysym, PageKeysUseKeysymdefNames) {
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_PRIOR, true), "Prior");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_NEXT, true), "Next");
}

TEST(VkKeysym, PrintableKeysUseUnshiftedBaseName) {
    EXPECT_EQ(evgetwindows::VkToKeysymName('A', false), "a");
    EXPECT_EQ(evgetwindows::VkToKeysymName('Z', false), "z");
    EXPECT_EQ(evgetwindows::VkToKeysymName('1', false), "1");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_F5, false), "F5");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_NUMPAD3, false), "KP_3");
}

TEST(VkKeysym, ExtendedReturnIsKeypadEnter) {
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_RETURN, false), "Return");
    EXPECT_EQ(evgetwindows::VkToKeysymName(VK_RETURN, true), "KP_Enter");
}

TEST(VkKeysym, EveryRepresentativeVkYieldsNonEmptyName) {
    std::vector<UINT> vks{
        VK_RETURN,   VK_SPACE,     VK_ESCAPE,   VK_TAB,     VK_BACK,     VK_LSHIFT,   VK_RSHIFT,
        VK_LCONTROL, VK_RCONTROL,  VK_LMENU,    VK_RMENU,   VK_LWIN,     VK_RWIN,     VK_CAPITAL,
        VK_NUMLOCK,  VK_SCROLL,    VK_PRIOR,    VK_NEXT,    VK_HOME,     VK_END,      VK_INSERT,
        VK_DELETE,   VK_LEFT,      VK_RIGHT,    VK_UP,      VK_DOWN,     VK_SNAPSHOT, VK_PAUSE,
        VK_APPS,     VK_MULTIPLY,  VK_ADD,      VK_SUBTRACT, VK_DECIMAL, VK_DIVIDE,   VK_OEM_1,
        VK_OEM_PLUS, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD, VK_OEM_2, VK_OEM_3,   VK_OEM_4,
        VK_OEM_5,    VK_OEM_6,     VK_OEM_7,    VK_VOLUME_UP, VK_MEDIA_PLAY_PAUSE,
    };
    for (UINT letter = 'A'; letter <= 'Z'; ++letter) {
        vks.push_back(letter);
    }
    for (UINT digit = '0'; digit <= '9'; ++digit) {
        vks.push_back(digit);
    }
    for (UINT fkey = VK_F1; fkey <= VK_F24; ++fkey) {
        vks.push_back(fkey);
    }
    for (UINT pad = VK_NUMPAD0; pad <= VK_NUMPAD9; ++pad) {
        vks.push_back(pad);
    }

    for (const UINT vk : vks) {
        EXPECT_FALSE(evgetwindows::VkToKeysymName(vk, false).empty()) << "empty name for vk " << vk;
    }
}

TEST(VkKeysym, DistinctKeysYieldDistinctNames) {
    const std::vector<UINT> vks{
        VK_RETURN, VK_SPACE, VK_ESCAPE, VK_TAB,  VK_BACK, VK_LSHIFT,
        VK_RSHIFT, VK_HOME,  VK_END,    'A',     'B',     '1',
    };
    std::set<std::string> names{};
    for (const UINT vk : vks) {
        names.insert(evgetwindows::VkToKeysymName(vk, false));
    }
    EXPECT_EQ(names.size(), vks.size());
}

TEST(VkKeysym, ResolveShiftFromScanCode) {
    constexpr USHORT kLeftShiftScan = 0x2A;
    constexpr USHORT kRightShiftScan = 0x36;

    const auto left = std::get<RAWKEYBOARD>(MakeKeyboard(VK_SHIFT, kLeftShiftScan, 0).data);
    const auto right = std::get<RAWKEYBOARD>(MakeKeyboard(VK_SHIFT, kRightShiftScan, 0).data);

    EXPECT_EQ(evgetwindows::ResolveVk(left), static_cast<UINT>(VK_LSHIFT));
    EXPECT_EQ(evgetwindows::ResolveVk(right), static_cast<UINT>(VK_RSHIFT));
}

TEST(VkKeysym, ResolveControlAndMenuFromExtendedBit) {
    const auto left_control = std::get<RAWKEYBOARD>(MakeKeyboard(VK_CONTROL, 0, 0).data);
    const auto right_control = std::get<RAWKEYBOARD>(MakeKeyboard(VK_CONTROL, 0, RI_KEY_E0).data);
    const auto left_menu = std::get<RAWKEYBOARD>(MakeKeyboard(VK_MENU, 0, 0).data);
    const auto right_menu = std::get<RAWKEYBOARD>(MakeKeyboard(VK_MENU, 0, RI_KEY_E0).data);

    EXPECT_EQ(evgetwindows::ResolveVk(left_control), static_cast<UINT>(VK_LCONTROL));
    EXPECT_EQ(evgetwindows::ResolveVk(right_control), static_cast<UINT>(VK_RCONTROL));
    EXPECT_EQ(evgetwindows::ResolveVk(left_menu), static_cast<UINT>(VK_LMENU));
    EXPECT_EQ(evgetwindows::ResolveVk(right_menu), static_cast<UINT>(VK_RMENU));
}
