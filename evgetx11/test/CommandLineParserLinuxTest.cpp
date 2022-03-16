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

#include "evgetcore/CommandLine/ParserLinux.h"
#include "TestUtils/CommandLine/CommandLineTestUtils.h"
#include <gtest/gtest.h>

namespace Cmd = CliOption;
namespace CmdUtils = TestUtils::CommandLineTestUtils;

using CommandLineParserLinuxTest = CmdUtils::CommandLineTest;

TEST_F(CommandLineParserLinuxTest, MouseDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-m", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CliOption::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getMouseDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, MouseDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--mouse-devices", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CliOption::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getMouseDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, KeyDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-k", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CliOption::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getKeyDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, KeyDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--key-devices", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CliOption::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getKeyDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, TouchDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-t", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CliOption::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getTouchDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, TouchDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--touch-devices", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CliOption::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getTouchDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, ListEventDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-l"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.isListEventDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, ListEventDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--list-devices"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.isListEventDevices());
    });
}

TEST_F(CommandLineParserLinuxTest, AtLeastOneDevicePresent) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_THROW(cmd.parseCommandLine(argc, argv), InvalidCommandLineOption);
    });
}

TEST_F(CommandLineParserLinuxTest, OnlyListDevicesPresent) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-l", "-m", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_THROW(cmd.parseCommandLine(argc, argv), InvalidCommandLineOption);
    });
}