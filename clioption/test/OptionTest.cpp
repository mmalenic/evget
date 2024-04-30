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

#include "CliOptionTestUtils.h"
#include "clioption/Error.h"
#include "clioption/Option.h"

namespace po = boost::program_options;
namespace Utils = CliOptionTestUtils;

TEST(OptionTest, RequiredPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a", "1"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<int>(desc).shortName('a').required().build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(option, parse, vm);
            ASSERT_EQ(1, option.getValue());
        }
    );
}

TEST(OptionTest, RequiredNotPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<int>(desc).shortName('a').required().build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            ASSERT_THROW(Utils::storeAndNotifyOption(option, parse, vm), po::required_option);
        }
    );
}

TEST(OptionTest, DefaultPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a", "1"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(2).build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(option, parse, vm);
            ASSERT_EQ(1, option.getValue());
        }
    );
}

TEST(OptionTest, DefaultNotPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(2).build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(option, parse, vm);
            ASSERT_EQ(2, option.getValue());
        }
    );
}

TEST(OptionTest, ImplicitValuePresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(2).implicitValue(1).build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(option, parse, vm);
            ASSERT_EQ(1, option.getValue());
        }
    );
}

TEST(OptionTest, ImplicitValueNotPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<int>(desc).shortName('a').required().build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            ASSERT_THROW(Utils::storeAndNotifyOption(option, parse, vm), po::invalid_command_line_syntax);
        }
    );
}

TEST(OptionTest, RepresentationOption) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(1).representation("repr").build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(option, parse, vm);
            ASSERT_EQ(1, option.getValue());
            ASSERT_EQ("repr", option.getRepresentation());
        }
    );
}

TEST(OptionTest, ConflictingOptionsNotPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a", "1"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').required().conflictsWith("b").build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').defaultValue(1).conflictsWith("a").build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(options, parse, vm);
            ASSERT_EQ(1, options[0].getValue());
            ASSERT_EQ(1, options[1].getValue());
        }
    );
}

TEST(OptionTest, ConflictingOptionsPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a", "1", "-b", "2"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').required().conflictsWith("b").build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').required().conflictsWith("a").build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            // todo
            // ASSERT_THROW(Utils::storeAndNotifyOption(options, parse, vm), CliOption::InvalidCommandLineOption);
        }
    );
}

TEST(OptionTest, ConflictingOptionsList) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a", "1", "-b", "2"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').required().conflictsWith({"b", "c"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').required().conflictsWith({"a", "c"}).build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            // todo
            // ASSERT_THROW(Utils::storeAndNotifyOption(options, parse, vm), CliOption::InvalidCommandLineOption);
        }
    );
}

TEST(OptionTest, AtLeastPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-b", "2"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}).build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(options, parse, vm);
            ASSERT_EQ(1, options[0].getValue());
            ASSERT_EQ(2, options[1].getValue());
        }
    );
}

TEST(OptionTest, AtLeastNotPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}).build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            // todo
            // ASSERT_THROW(Utils::storeAndNotifyOption(options, parse, vm), CliOption::InvalidCommandLineOption);
        }
    );
}

TEST(OptionTest, AtLeastNonList) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a", "2"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast("b").build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast("a").build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(options, parse, vm);
            ASSERT_EQ(2, options[0].getValue());
            ASSERT_EQ(1, options[1].getValue());
        }
    );
}

TEST(OptionTest, AtLeastExceptPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-d", "2"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}, {"d"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}, {"d"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('d').defaultValue(1).build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(options, parse, vm);
            ASSERT_EQ(1, options[0].getValue());
            ASSERT_EQ(1, options[1].getValue());
            ASSERT_EQ(2, options[2].getValue());
        }
    );
}

TEST(OptionTest, AtLeastExceptNotPresent) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast({"b", "c"}, {"d"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast({"a", "c"}, {"d"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('d').defaultValue(1).build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            // todo
            // ASSERT_THROW(Utils::storeAndNotifyOption(options, parse, vm), CliOption::InvalidCommandLineOption);
        }
    );
}

TEST(OptionTest, AtLeastExceptNonList) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-d", "2"},
        [](po::options_description& desc) {
            return std::vector{
                CliOption::OptionBuilder<int>(desc).shortName('a').defaultValue(1).atLeast("b", {"d"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('b').defaultValue(1).atLeast("a", {"d"}).build(),
                CliOption::OptionBuilder<int>(desc).shortName('d').defaultValue(1).build()};
        },
        [](po::variables_map& vm, auto& options, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(options, parse, vm);
            ASSERT_EQ(1, options[0].getValue());
            ASSERT_EQ(1, options[1].getValue());
            ASSERT_EQ(2, options[2].getValue());
        }
    );
}

TEST(OptionTest, Multitoken) {  // NOLINT(cert-err58-cpp)
    Utils::withOption(
        {"program", "-a", "1", "2"},
        [](po::options_description& desc) {
            return CliOption::OptionBuilder<std::vector<int>>(desc).shortName('a').required().multitoken().build();
        },
        [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
            Utils::storeAndNotifyOption(option, parse, vm);
            std::vector<int> values = option.getValue();
            ASSERT_TRUE(std::find(values.begin(), values.end(), 1) != values.end());
            ASSERT_TRUE(std::find(values.begin(), values.end(), 2) != values.end());
        }
    );
}