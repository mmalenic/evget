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

#include <gmock/gmock.h>
#include <CommandLine/CommandLineTestUtilities.h>

namespace Cmd = CommandLine;
namespace CmdUtils = TestUtilities::CommandLineTestUtilities;

using CommandLineTest = CmdUtils::CommandLineTest;

TEST_F(CommandLineTest, ParseCommandLine) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, HelpOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-h"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(false, cmd.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, VersionOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-v"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(false, cmd.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, ConfigOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-c", "config"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"config"}, cmd.getConfigFile());
    });
}

TEST_F(CommandLineTest, FolderOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-o", "folder"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"folder"}, cmd.getFolder());
    });
}

TEST_F(CommandLineTest, FiletypeOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-t", "sqlite"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(std::vector<Cmd::Filetype>{Cmd::sqlite}, cmd.getFiletype());
    });
}

TEST_F(CommandLineTest, PrintOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-p"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.shouldPrint());
    });
}

TEST_F(CommandLineTest, SystemEventsOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-s"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.useSystemEvents());
    });
}

TEST_F(CommandLineTest, LogLevelOption) { // NOLINT(cert-err58-cpp)
    Cmd::Parser cmd{""};
    CmdUtils::makeCmd({"program", "-u", "err"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(spdlog::level::err, cmd.getLogLevel());
    });
}