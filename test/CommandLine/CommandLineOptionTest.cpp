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
#include <CommandLine/CommandLineOption.h>

namespace po = boost::program_options;

TEST(CommandLineOptionTest, RequiredImplicitOrDefault) { // NOLINT(cert-err58-cpp)
    po::options_description desc{};
    ASSERT_THROW(CommandLineOptionBuilder<int>(desc).shortName("name").build(), UnsupportedOperationException);
}

TEST(CommandLineOptionTest, FlagNotPresent) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program"}, [](po::options_description& desc) {
        return CommandLineOptionBuilder<bool>(desc).shortName("a").build();
    }, [](po::variables_map& vm, CommandLineOption<bool>& option) {
        option.afterRead(vm);
        ASSERT_EQ(false, option.getValue());
    });
}

TEST(CommandLineOptionTest, FlagPresent) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a"}, [](po::options_description& desc) {
        return CommandLineOptionBuilder<bool>(desc).shortName("a").build();
    }, [](po::variables_map& vm, CommandLineOption<bool>& option) {
        option.afterRead(vm);
        ASSERT_EQ(true, option.getValue());
    });
}

TEST(CommandLineOptionTest, CheckRequired) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program"}, [](po::options_description& desc) {
        return CommandLineOptionBuilder<int>(desc).shortName("a").required().build();
    }, [](po::variables_map& vm, CommandLineOption<int>& option) {
        ASSERT_THROW(option.afterRead(vm), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, ImplicitValuePresent) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a"}, [](po::options_description& desc) {
        return CommandLineOptionBuilder<int>(desc).shortName("a").implicitValue(1).build();
    }, [](po::variables_map& vm, CommandLineOption<int>& option) {
        option.afterRead(vm);
        ASSERT_EQ(1, option.getValue());
    });
}

TEST(CommandLineOptionTest, ImplicitValueNotPresent) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a"}, [](po::options_description& desc) {
        return CommandLineOptionBuilder<int>(desc).shortName("a").required().build();
    }, [](po::variables_map& vm, CommandLineOption<int>& option) {
        ASSERT_THROW(option.afterRead(vm), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, ConflictingOptions) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a", "1", "-b", "2"}, [](po::options_description& desc) {
        return CommandLineOptionBuilder<int>(desc).shortName("a").required().conflictsWith("b").build();
    }, [](po::variables_map& vm, CommandLineOption<int>& option) {
        ASSERT_THROW(option.afterRead(vm), InvalidCommandLineOption);
    });
}

TEST(CommandLineOptionTest, ParseValue) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a", "1"}, [](po::options_description& desc) {
        return CommandLineOptionBuilder<int>(desc).shortName("a").required().build();
    }, [](po::variables_map& vm, CommandLineOption<int>& option) {
        option.afterRead(vm);
        ASSERT_EQ(1, option.getValue());
    });
}