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

TEST(X11Test, ReplaceInvalidUtf8) {
    ASSERT_EQ(evgetx11::X11::ReplaceInvalidUtf8(""), "");
    ASSERT_EQ(evgetx11::X11::ReplaceInvalidUtf8("title"), "title");
    ASSERT_EQ(
        evgetx11::X11::ReplaceInvalidUtf8("caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80"),
        "caf\xc3\xa9 \xe2\x82\xac \xf0\x9f\x98\x80"
    );
}

TEST(X11Test, ReplaceInvalidUtf8InvalidByte) {
    ASSERT_EQ(evgetx11::X11::ReplaceInvalidUtf8("a\xe9z"), "a\xef\xbf\xbdz");
    ASSERT_EQ(evgetx11::X11::ReplaceInvalidUtf8("\xc3"), "\xef\xbf\xbd");
    ASSERT_EQ(evgetx11::X11::ReplaceInvalidUtf8("\xc3!"), "\xef\xbf\xbd!");
    ASSERT_EQ(evgetx11::X11::ReplaceInvalidUtf8("\xc0\xaf"), "\xef\xbf\xbd\xef\xbf\xbd");
    ASSERT_EQ(evgetx11::X11::ReplaceInvalidUtf8("\xed\xa0\x80"), "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd");
}
