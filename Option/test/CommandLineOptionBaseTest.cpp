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
#include "../include/Option/Option.h"
#include "CommandLine/CommandLineTestUtilities.h"

namespace po = boost::program_options;
namespace CmdUtils = TestUtilities::CommandLineTestUtilities;
namespace Cmd = CommandLine;

TEST(CommandLineOptionBaseTest, GetShortName) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).required().shortName('n').build();
    ASSERT_EQ("n", option.getShortName());
}

TEST(CommandLineOptionBaseTest, GetLongName) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).required().longName("name").build();
    ASSERT_EQ("name", option.getLongName());
}

TEST(CommandLineOptionBaseTest, GetDescription) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').required().description("desc").build();
    ASSERT_EQ("desc", option.getDescription());
}

TEST(CommandLineOptionBaseTest, GetNameShort) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).required().shortName('n').build();
    ASSERT_EQ("n", option.getName());
}

TEST(CommandLineOptionBaseTest, GetNameLong) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).required().longName("name").build();
    ASSERT_EQ("name", option.getName());
}

TEST(CommandLineOptionBaseTest, GetNameBoth) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).required().shortName('n').longName("name").build();
    ASSERT_EQ("name", option.getName());
}

TEST(CommandLineOptionBaseTest, SetValue) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').defaultValue(1).build();
    option.setValue(2);
    ASSERT_EQ(2, option.getValue());
}

TEST(CommandLineOptionBaseTest, GetDefaultValue) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').defaultValue(1).build();
    ASSERT_EQ(1, option.getValue());
}

TEST(CommandLineOptionBaseTest, GetOptionalDefaultValue) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').defaultValue(1).build();
    ASSERT_EQ(1, option.getDefaultValue());
}

TEST(CommandLineOptionBaseTest, GetOptionalImplicitValue) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').required().implicitValue(1).build();
    ASSERT_EQ(1, option.getImplicitValue());
}

TEST(CommandLineOptionBaseTest, GetRepresentation) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').required().representation("repr").build();
    ASSERT_EQ("repr", option.getRepresentation());
}

TEST(CommandLineOptionBaseTest, IsRequired) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').required().build();
    ASSERT_EQ(true, option.isRequired());
}

TEST(CommandLineOptionBaseTest, RunShortOnly) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a", "1"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').required().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionBaseTest, RunLongOnly) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "--name", "1"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).longName("name").required().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionBaseTest, RunBothShortPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a", "1"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').longName("name").required().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionBaseTest, RunBothLongPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "--name", "1"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').longName("name").required().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionBaseTest, PositionalOptionPresent) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    po::positional_options_description posDesc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').required().positional(1, posDesc).build();

    CmdUtils::makeCmd({"program", "1"}, [&desc, &posDesc, &option](int argc, const char** argv) {
        po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc).positional(posDesc);
        CmdUtils::storeAndNotifyOption(option, parse, {});
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionBaseTest, PositionalOptionNotPresent) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    po::positional_options_description posDesc{};
    Cmd::Option<int> option = Cmd::OptionBuilder<int>(desc).shortName('n').defaultValue(1).positional(1, posDesc).build();

    CmdUtils::makeCmd({"program"}, [&desc, &posDesc, &option](int argc, const char** argv) {
        po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc).positional(posDesc);
        CmdUtils::storeAndNotifyOption(option, parse, {});
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionBaseTest, NoDefaultAndNotRequired) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    ASSERT_THROW(Cmd::OptionBuilder<int>(desc).shortName('a').build(), UnsupportedOperationException);
}
