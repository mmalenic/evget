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
#include <CommandLine/Option.h>
#include <CommandLine/CommandLineTestUtilities.h>
#include "CommandLine/Parser.h"

namespace po = boost::program_options;
namespace CmdUtils = TestUtilities::CommandLineTestUtilities;
namespace Cmd = CommandLine;

TEST(CommandLineOptionBaseTest, GetShortNameTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).required().shortName("name").build();
    ASSERT_EQ("name", option.getShortName());
}

TEST(CommandLineOptionBaseTest, GetLongNameTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).required().longName("name").build();
    ASSERT_EQ("name", option.getLongName());
}

TEST(CommandLineOptionBaseTest, GetDescriptionTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName("name").required().description("cmdDesc").build();
    ASSERT_EQ("cmdDesc", option.getDescription());
}

TEST(CommandLineOptionBaseTest, SetValue) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName("name").required().defaultValue(1).build();
    option.setValue(2);
    ASSERT_EQ(2, option.getValue());
}

TEST(CommandLineOptionBaseTest, GetDefaultValueTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName("name").required().defaultValue(1).build();
    ASSERT_EQ(1, option.getValue());
}

TEST(CommandLineOptionBaseTest, PositionalOptionTest) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    po::positional_options_description posDesc{};
    po::variables_map vm{};

    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName("n").longName("name").required().positional(1, posDesc).build();

    CmdUtils::makeCmd({"program", "1"}, [&desc, &posDesc, &vm, &option](int argc, const char** argv) {
        po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc).positional(posDesc);
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
    });
}
