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
#include <TestUtils/CommandLine/CommandLineTestUtils.h>

namespace Cmd = CommandLine;
namespace CmdUtils = TestUtils::CommandLineTestUtils;

using CommandLineParserTest = CmdUtils::CommandLineTest;

TEST_F(CommandLineParserTest, ParseCommandLine) { // NOLINT(cert-err58-cpp)
    CmdUtils::makeCmd({"program"}, [](int argc, const char* argv[]) {
        ASSERT_EQ(true, CmdUtils::MockParser{}.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineParserTest, HelpOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-h"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(false, cmd.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineParserTest, HelpOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--help"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(false, cmd.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineParserTest, VersionOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-v"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(false, cmd.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineParserTest, VersionOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--version"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(false, cmd.parseCommandLine(argc, argv));
    });
}

TEST_F(CommandLineParserTest, ConfigOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-c", "config"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"config"}, cmd.getConfigFile());
    });
}

TEST_F(CommandLineParserTest, ConfigOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--config", "config"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"config"}, cmd.getConfigFile());
    });
}

TEST_F(CommandLineParserTest, FolderOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-o", "folder"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"folder"}, cmd.getFolder());
    });
}

TEST_F(CommandLineParserTest, FolderOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--folder", "folder"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(Cmd::fs::path{"folder"}, cmd.getFolder());
    });
}

TEST_F(CommandLineParserTest, FiletypesOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-f", "sqlite"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(std::vector<Cmd::Filetype>{Cmd::sqlite}, cmd.getFiletype());
    });
}

TEST_F(CommandLineParserTest, FiletypesOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--filetypes", "sqlite"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(std::vector<Cmd::Filetype>{Cmd::sqlite}, cmd.getFiletype());
    });
}

TEST_F(CommandLineParserTest, PrintOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-p"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.shouldPrint());
    });
}

TEST_F(CommandLineParserTest, PrintOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--print"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.shouldPrint());
    });
}

TEST_F(CommandLineParserTest, SystemEventsOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-s"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.useSystemEvents());
    });
}

TEST_F(CommandLineParserTest, SystemEventsOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--use-system-events"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(true, cmd.useSystemEvents());
    });
}

TEST_F(CommandLineParserTest, LogLevelOptionShort) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "-u", "err"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(spdlog::level::err, cmd.getLogLevel());
    });
}

TEST_F(CommandLineParserTest, LogLevelOptionLong) { // NOLINT(cert-err58-cpp)
    CmdUtils::MockParser cmd{};
    CmdUtils::makeCmd({"program", "--log-level", "err"}, [&cmd](int argc, const char* argv[]) {
        ASSERT_EQ(true, cmd.parseCommandLine(argc, argv));
        ASSERT_EQ(spdlog::level::err, cmd.getLogLevel());
    });
}