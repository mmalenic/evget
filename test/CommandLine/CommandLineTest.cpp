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
    CmdUtils::makeCmd({"program"}, [](int argc, const char* argv[]) {
        ASSERT_EQ(true, Cmd::Parser{""}.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, HelpOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-h"}, [](int argc, const char* argv[]) {
        ASSERT_EQ(false, Cmd::Parser{""}.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, HelpOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--help"}, [](int argc, const char* argv[]) {
        ASSERT_EQ(false, Cmd::Parser{""}.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, VersionOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-v"}, [](int argc, const char* argv[]) {
        ASSERT_EQ(false, Cmd::Parser{""}.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, VersionOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--version"}, [](int argc, const char* argv[]) {
        ASSERT_EQ(false, Cmd::Parser{""}.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineTest, ConfigOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-c", "config"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"config"}, cmd.getConfigFile());
    });
}

TEST_F(CommandLineTest, ConfigOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--config", "config"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"config"}, cmd.getConfigFile());
    });
}

TEST_F(CommandLineTest, FolderOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-o", "folder"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"folder"}, cmd.getFolder());
    });
}

TEST_F(CommandLineTest, FolderOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--folder", "folder"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"folder"}, cmd.getFolder());
    });
}

TEST_F(CommandLineTest, FiletypesOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-t", "sqlite"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(std::vector<Cmd::Filetype>{Cmd::sqlite}, cmd.getFiletype());
    });
}

TEST_F(CommandLineTest, FiletypesOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--filetypes", "sqlite"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(std::vector<Cmd::Filetype>{Cmd::sqlite}, cmd.getFiletype());
    });
}

TEST_F(CommandLineTest, PrintOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-p"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.shouldPrint());
    });
}

TEST_F(CommandLineTest, PrintOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--print"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.shouldPrint());
    });
}

TEST_F(CommandLineTest, SystemEventsOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-s"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.useSystemEvents());
    });
}

TEST_F(CommandLineTest, SystemEventsOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--use-system-events"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.useSystemEvents());
    });
}

TEST_F(CommandLineTest, LogLevelOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "-u", "err"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(spdlog::level::err, cmd.getLogLevel());
    });
}

TEST_F(CommandLineTest, LogLevelOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program", "--log-level", "err"}, [](int argc, const char* argv[]) {
        Cmd::Parser cmd{""};
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(spdlog::level::err, cmd.getLogLevel());
    });
}