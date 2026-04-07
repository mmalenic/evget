// NOLINTBEGIN(bugprone-unchecked-optional-access)
#include "evgetlibinput/xkbcommon.h"

#include <gtest/gtest.h>

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-names.h>
#include <xkbcommon/xkbcommon.h>

#include "common/test_helpers.h"

namespace {

using test::kUsLayout;
using test::XkbKey;

} // namespace

TEST(XkbCommonTest, GetKeyName) {
    auto xkb = evgetlibinput::XkbCommon::NewWithNames(&kUsLayout);
    ASSERT_TRUE(xkb.has_value());

    auto name = xkb.value().GetKeyName(XkbKey(KEY_A));
    ASSERT_TRUE(name.has_value());
    ASSERT_EQ(*name, "a");
}

TEST(XkbCommonTest, GetKeyCharacter) {
    auto xkb = evgetlibinput::XkbCommon::NewWithNames(&kUsLayout);
    ASSERT_TRUE(xkb.has_value());

    auto character = xkb.value().GetKeyCharacter(XkbKey(KEY_A));
    ASSERT_TRUE(character.has_value());
    ASSERT_EQ(*character, "a");
}

TEST(XkbCommonTest, GetKeyNameNullopt) {
    auto xkb = evgetlibinput::XkbCommon::NewWithNames(&kUsLayout);
    ASSERT_TRUE(xkb.has_value());

    auto name = xkb.value().GetKeyName(0);
    ASSERT_FALSE(name.has_value());
}

TEST(XkbCommonTest, ModifierState) {
    auto xkb = evgetlibinput::XkbCommon::NewWithNames(&kUsLayout);
    ASSERT_TRUE(xkb.has_value());

    ASSERT_FALSE(xkb.value().IsModifierActive(XKB_MOD_NAME_SHIFT));
    ASSERT_FALSE(xkb.value().IsModifierActive(XKB_MOD_NAME_CTRL));

    xkb.value().UpdateKeyState(XkbKey(KEY_LEFTSHIFT), XKB_KEY_DOWN);
    ASSERT_TRUE(xkb.value().IsModifierActive(XKB_MOD_NAME_SHIFT));

    xkb.value().UpdateKeyState(XkbKey(KEY_LEFTSHIFT), XKB_KEY_UP);
    ASSERT_FALSE(xkb.value().IsModifierActive(XKB_MOD_NAME_SHIFT));
}

TEST(XkbCommonTest, GetKeyCharacterShift) {
    auto xkb = evgetlibinput::XkbCommon::NewWithNames(&kUsLayout);
    ASSERT_TRUE(xkb.has_value());

    xkb.value().UpdateKeyState(XkbKey(KEY_LEFTSHIFT), XKB_KEY_DOWN);

    auto name = xkb.value().GetKeyName(XkbKey(KEY_A));
    ASSERT_TRUE(name.has_value());
    ASSERT_EQ(*name, "A");

    auto character = xkb.value().GetKeyCharacter(XkbKey(KEY_A));
    ASSERT_TRUE(character.has_value());
    ASSERT_EQ(*character, "A");

    xkb.value().UpdateKeyState(XkbKey(KEY_LEFTSHIFT), XKB_KEY_UP);

    auto character_lower = xkb.value().GetKeyCharacter(XkbKey(KEY_A));
    ASSERT_TRUE(character_lower.has_value());
    ASSERT_EQ(*character_lower, "a");
}

// NOLINTEND(bugprone-unchecked-optional-access)
