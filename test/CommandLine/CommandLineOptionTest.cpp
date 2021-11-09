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
#include <CommandLine/CommandLineTestUtilities.h>
#include <CommandLine/Option.h>

namespace po = boost::program_options;
namespace Cmd = CommandLine;

TEST(CommandLineOptionTest, UnsupportedBuilderUse) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    ASSERT_THROW(Cmd::OptionBuilder<int>(desc).shortName("a").build(), UnsupportedOperationException);
}

TEST(CommandLineOptionTest, CheckRequired) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program"}, [](po::options_description& desc) {
        return Cmd::OptionBuilder<int>(desc).shortName("a").required().build();
    }, [](po::variables_map& vm, auto& option, po::command_line_parser& parser) {
        ASSERT_THROW(Cmd::Option<int>::runFor({option}, vm, parser), po::required_option);
    });
}

TEST(CommandLineOptionTest, ImplicitValuePresent) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a"}, [](po::options_description& desc) {
        return Cmd::OptionBuilder<int>(desc).shortName("a").defaultValue(2).implicitValue(1).build();
    }, [](po::variables_map& vm, auto& option, po::command_line_parser& parser) {
        Cmd::Option<int>::runFor({option}, vm, parser);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionTest, ImplicitValueNotPresent) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a"}, [](po::options_description& desc) {
        return Cmd::OptionBuilder<int>(desc).shortName("a").required().build();
    }, [](po::variables_map& vm, auto& option, po::command_line_parser& parser) {
        ASSERT_THROW(Cmd::Option<int>::runFor({option}, vm, parser), po::invalid_command_line_syntax);
    });
}

TEST(CommandLineOptionTest, ConflictingOptions) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    po::variables_map vm{};

    auto optionA = Cmd::OptionBuilder<int>(desc).shortName("a").required().conflictsWith("b").build();
    auto optionB = Cmd::OptionBuilder<int>(desc).shortName("b").required().build();

    TestUtilities::CommandLineTestUtilities::makeCmd({"program", "-a", "1", "-b", "2"}, [&desc, &vm, &optionA, &optionB](int argc, const char** argv) {
        po::command_line_parser parser = po::command_line_parser(argc, argv).options(desc);
        ASSERT_THROW(Cmd::Option<int>::runFor({optionA, optionB}, vm, parser), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, ParseValue) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a", "1"}, [](po::options_description& desc) {
        return Cmd::OptionBuilder<int>(desc).shortName("a").required().build();
    }, [](po::variables_map& vm, auto& option, po::command_line_parser& parser) {
        Cmd::Option<int>::runFor({option}, vm, parser);
        ASSERT_EQ(1, option.getValue());
    });
}