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

namespace po = boost::program_options;
namespace CmdUtils = TestUtilities::CommandLineTestUtilities;
namespace Cmd = CommandLine;

TEST(CommandLineOptionTest, CheckRequired) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').required().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        ASSERT_THROW(CmdUtils::storeAndNotifyOption(option, parse, vm), po::required_option);
    });
}

TEST(CommandLineOptionTest, DefaultValue) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').defaultValue(2).build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(2, option.getValue());
    });
}

TEST(CommandLineOptionTest, ImplicitValuePresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').defaultValue(2).implicitValue(1).build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionTest, ImplicitValueNotPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').required().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        ASSERT_THROW(CmdUtils::storeAndNotifyOption(option, parse, vm), po::invalid_command_line_syntax);
    });

}

TEST(CommandLineOptionTest, RepresentationOption) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').defaultValue(1).representation("repr").build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
        ASSERT_EQ("repr", option.getRepresentation());
    });
}

TEST(CommandLineOptionTest, ConflictingOptions) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a", "1", "-b", "2"}, [](po::options_description &desc) {
        return std::vector{
                Cmd::OptionBuilder<int>(desc).shortName('a').required().conflictsWith("b").build(),
                Cmd::OptionBuilder<int>(desc).shortName('b').required().conflictsWith("a").build()
        };
    }, [](po::variables_map &vm, auto &options, po::command_line_parser &parse) {
        ASSERT_THROW(CmdUtils::storeAndNotifyOption(options, parse, vm), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, ConflictingOptionsList) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a", "1", "-b", "2"}, [](po::options_description &desc) {
        return std::vector{
                Cmd::OptionBuilder<int>(desc).shortName('a').required().conflictsWith({"b", "c"}).build(),
                Cmd::OptionBuilder<int>(desc).shortName('b').required().conflictsWith({"a", "c"}).build()
        };
    }, [](po::variables_map &vm, auto &options, po::command_line_parser &parse) {
        ASSERT_THROW(CmdUtils::storeAndNotifyOption(options, parse, vm), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, AtLeastPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-b", "2"}, [](po::options_description &desc) {
        return std::vector{
                Cmd::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}).build(),
                Cmd::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}).build()
        };
    }, [](po::variables_map &vm, auto &options, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(options, parse, vm);
        ASSERT_EQ(1, options[0].getValue());
        ASSERT_EQ(2, options[1].getValue());
    });
}

TEST(CommandLineOptionTest, AtLeastNotPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program"}, [](po::options_description &desc) {
        return std::vector{
                Cmd::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}).build(),
                Cmd::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}).build()
        };
    }, [](po::variables_map &vm, auto &options, po::command_line_parser &parse) {
        ASSERT_THROW(CmdUtils::storeAndNotifyOption(options, parse, vm), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, AtLeastExceptPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-d", "2"}, [](po::options_description &desc) {
        return std::vector{
                Cmd::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}, {"d"}).build(),
                Cmd::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}, {"d"}).build(),
                Cmd::OptionBuilder<int>(desc).shortName('d').defaultValue(1).build()
        };
    }, [](po::variables_map &vm, auto &options, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(options, parse, vm);
        ASSERT_EQ(1, options[0].getValue());
        ASSERT_EQ(1, options[1].getValue());
        ASSERT_EQ(2, options[2].getValue());
    });
}

TEST(CommandLineOptionTest, AtLeastExceptNotPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program"}, [](po::options_description &desc) {
        return std::vector{
            Cmd::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}, {"d"}).build(),
            Cmd::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}, {"d"}).build(),
            Cmd::OptionBuilder<int>(desc).shortName('d').defaultValue(1).build()
        };
    }, [](po::variables_map &vm, auto &options, po::command_line_parser &parse) {
        ASSERT_THROW(CmdUtils::storeAndNotifyOption(options, parse, vm), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, ParseValue) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a", "1"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<int>(desc).shortName('a').required().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionTest, Multitoken) { // NOLINT(cert-err58-cpp)
    CmdUtils::withOption({"program", "-a", "1", "2"}, [](po::options_description &desc) {
        return Cmd::OptionBuilder<std::vector<int>>(desc).shortName('a').required().multitoken().build();
    }, [](po::variables_map &vm, auto &option, po::command_line_parser &parse) {
        CmdUtils::storeAndNotifyOption(option, parse, vm);
        std::vector<int> values = option.getValue();
        ASSERT_TRUE(std::find(values.begin(), values.end(), 1) != values.end());
        ASSERT_TRUE(std::find(values.begin(), values.end(), 2) != values.end());
    });
}