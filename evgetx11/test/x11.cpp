#include <gtest/gtest.h>

#include <string>

// clang-format off
#include "evgetx11/x11.h"
// clang-format on

TEST(X11Test, Latin1ToUtf8) {
    ASSERT_EQ(evgetx11::X11::Latin1ToUtf8(""), "");
    ASSERT_EQ(evgetx11::X11::Latin1ToUtf8("a2~"), "a2~");
    ASSERT_EQ(evgetx11::X11::Latin1ToUtf8("\x7f"), "\x7f");
    ASSERT_EQ(evgetx11::X11::Latin1ToUtf8("\x80"), "\xc2\x80");
    ASSERT_EQ(evgetx11::X11::Latin1ToUtf8("\xe9"), "\xc3\xa9");
    ASSERT_EQ(evgetx11::X11::Latin1ToUtf8("\xff"), "\xc3\xbf");
    ASSERT_EQ(evgetx11::X11::Latin1ToUtf8(std::string{"a\xe9z"}), "a\xc3\xa9z");
}
