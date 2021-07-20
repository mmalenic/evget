// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "CommandLineLinux.h"

#include <gtest/gtest.h>

TEST(CommandLineTest, HelpOptionShort) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "-h";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.read_args(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, HelpOptionLong) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "--help";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.read_args(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, VersionOptionShort) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "-v";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.read_args(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, VersionOptionLong) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "--version";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.read_args(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, ListEventDevicesExclusive) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg0[] = "--list-event-devices";
    char arg1[] = "--mouse-device";
    char arg2[] = "/";
    char *argv[] = { &name[0], &arg0[0], &arg1[0], &arg2[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.read_args(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, AtLeastOneEventDevice) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    cmd.read_args();
    EXPECT_EXIT(cmd.read_args(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}