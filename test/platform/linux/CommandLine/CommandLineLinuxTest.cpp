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

#include "CommandLine/ParserLinux.h"
#include "CommandLine/CommandLineTestUtilities.h"
#include <gtest/gtest.h>

namespace Cmd = CommandLine;
namespace CmdUtils = TestUtilities::CommandLineTestUtilities;

using CommandLineLinuxTest = CmdUtils::CommandLineTest;

TEST_F(CommandLineLinuxTest, MouseDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-m", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CommandLine::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getMouseDevices());
    });
}

TEST_F(CommandLineLinuxTest, MouseDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--mouse-devices", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CommandLine::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getMouseDevices());
    });
}

TEST_F(CommandLineLinuxTest, KeyDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-k", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CommandLine::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getKeyDevices());
    });
}

TEST_F(CommandLineLinuxTest, KeyDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--key-devices", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CommandLine::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getKeyDevices());
    });
}

TEST_F(CommandLineLinuxTest, TouchDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-t", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CommandLine::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getTouchDevices());
    });
}

TEST_F(CommandLineLinuxTest, TouchDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--touch-devices", "/dev/input/event0"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        std::vector<CommandLine::fs::path> expected = {{"/dev/input/event0"}};
        ASSERT_EQ(expected, cmd.getTouchDevices());
    });
}

TEST_F(CommandLineLinuxTest, ListEventDevicesOptionShort) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "-l"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.isListEventDevices());
    });
}

TEST_F(CommandLineLinuxTest, ListEventDevicesOptionLong) { // NOLINT(cert-err58-cpp)
    Cmd::ParserLinux cmd{};
    CmdUtils::makeCmd({"program", "--list-devices"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.isListEventDevices());
    });
}