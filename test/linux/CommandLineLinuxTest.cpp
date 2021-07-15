// Copyright (C) Marko Malenic 2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <gtest/gtest.h>
#include <CommandLineLinux.h>

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
    EXPECT_EXIT(cmd.read_args(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}