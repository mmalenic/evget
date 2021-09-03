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
#include <CommandLine.h>

/**
 * Mock command line object.
 */
class MockCommandLine : public CommandLine {
public:
    MOCK_METHOD(void, readArgs, (), (override));
    MOCK_METHOD(std::string, platformInformation, (), (override));
};

/**
 * Make the command line object.
 * @param args args to use
 * @param create_cmd function to create the object
 */
void make_cmd(std::initializer_list<char *> args, auto&& create_cmd) {
    std::vector<char *> vector{args};
    vector.push_back(nullptr);
    char **argv = &vector[0];
    int argc = static_cast<int>(vector.size() - 1);
    create_cmd(argc, argv);
}

void assert_exit(CommandLine& cmd) {
    ASSERT_EXIT(cmd.readArgs(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, HelpOptionShort) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "-h";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.readArgs(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, HelpOptionLong) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "--help";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.readArgs(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, VersionOptionShort) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "-v";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.readArgs(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}

TEST(CommandLineTest, VersionOptionLong) { // NOLINT(cert-err58-cpp)
    char name[] = "";
    char arg[] = "--version";
    char *argv[] = { &name[0], &arg[0], nullptr };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
    CommandLineLinux cmd { argc, argv };
    EXPECT_EXIT(cmd.readArgs(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}