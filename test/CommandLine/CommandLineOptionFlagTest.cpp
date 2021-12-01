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
#include <CommandLine/OptionFlag.h>
#include "CommandLine/Parser.h"

namespace po = boost::program_options;
namespace CmdUtils = TestUtilities::CommandLineTestUtilities;
namespace Cmd = CommandLine;

TEST(CommandLineOptionFlagTest, FlagNotPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::assertOnCmd({"program"}, [](po::options_description& desc) {
        return Cmd::OptionBuilder<bool>(desc).shortName("a").buildFlag();
    }, [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
        CmdUtils::storeAndNotify(option, parse, vm);
        ASSERT_FALSE(option.getValue());
    });
}

TEST(CommandLineOptionFlagTest, FlagPresent) { // NOLINT(cert-err58-cpp)
    CmdUtils::assertOnCmd({"program", "-a"}, [](po::options_description& desc) {
        return Cmd::OptionBuilder<bool>(desc).shortName("a").buildFlag();
    }, [](po::variables_map& vm, auto& option, po::command_line_parser& parse) {
        CmdUtils::storeAndNotify(option, parse, vm);
        ASSERT_TRUE(option.getValue());
    });
}