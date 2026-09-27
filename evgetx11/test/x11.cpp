#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>

#include <string>
#include <vector>

#include "common/log.h"
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

TEST(X11Test, OnErrorLogs) {
    const test::CapturedLog log{};
    XErrorEvent window_error{
        .type = 0,
        .display = nullptr,
        .resourceid = 1,
        .serial = 1,
        .error_code = BadWindow,
        .request_code = X_GetWindowAttributes,
        .minor_code = 0
    };
    XErrorEvent other_window{window_error};
    other_window.resourceid = 2;
    XErrorEvent atom_error{window_error};
    atom_error.error_code = BadAtom;
    atom_error.request_code = X_GetAtomName;

    ASSERT_EQ(evgetx11::X11::OnError(nullptr, &window_error), 0);
    ASSERT_EQ(evgetx11::X11::OnError(nullptr, &other_window), 0);
    ASSERT_EQ(evgetx11::X11::OnError(nullptr, &atom_error), 0);

    ASSERT_EQ(
        log.Lines(),
        (std::vector<std::string>{
            "warning X request 3.0 failed: error 3",
            "debug X request 3.0 failed: error 3",
            "warning X request 17.0 failed: error 5",
        })
    );
}
