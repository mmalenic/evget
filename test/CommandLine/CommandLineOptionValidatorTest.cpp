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
namespace Cmd = CommandLine;

TEST(CommandLineOptionTest, ParseValidatedValue) { // NOLINT(cert-err58-cpp)
    TestUtilities::CommandLineTestUtilities::assertOnCmd({"program", "-a", "1"}, [](po::options_description& desc) {
        return Cmd::OptionBuilder<int>(desc).shortName("a").required().build([](const std::string& _) {
            return 2;
        });
    }, [](po::variables_map& vm, Cmd::OptionValidator<int>& option) {
        option.afterRead(vm);
        ASSERT_EQ(2, option.getValue());
    });
}