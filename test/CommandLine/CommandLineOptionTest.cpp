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

#include <gtest/gtest.h>
#include <CommandLine/CommandLineOption.h>
#include "CommandLineTest.cpp"
#include "CommandLine/CommandLineUtilities.h"

namespace po = boost::program_options;

TEST(CommandLineOptionTest, GetShortNameTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    CommandLineOption<int> option = CommandLineOptionBuilder<int>(desc).required().shortName("name").build();
    ASSERT_EQ("name", option.getShortName());
}

TEST(CommandLineOptionTest, GetLongNameTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    CommandLineOption<int> option = CommandLineOptionBuilder<int>(desc).required().longName("name").build();
    ASSERT_EQ("name", option.getLongName());
}

TEST(CommandLineOptionTest, GetDescriptionTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    CommandLineOption<int> option = CommandLineOptionBuilder<int>(desc).required().description("desc").build();
    ASSERT_EQ("desc", option.getDescription());
}

TEST(CommandLineOptionTest, GetDefaultValueTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    CommandLineOption<int> option = CommandLineOptionBuilder<int>(desc).required().defaultValue(1).build();
    ASSERT_EQ(1, option.getValue());
}

TEST(CommandLineOptionTest, CheckRequired) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    po::variables_map vm{};
    CommandLineOption<int> option = CommandLineOptionBuilder<int>(desc).shortName("name").required().build();

    TestUtilities::CommandLineUtilities::makeCmd({"program"}, [&desc, &vm](int argc, const char* argv[]) {
        store(parse_command_line(argc, argv, desc), vm);
        vm.notify();
    });
    ASSERT_THROW(option.afterRead(vm), InvalidCommandLineOption);
}