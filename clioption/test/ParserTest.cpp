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
#include "clioption/Parser.h"
#include "clioption/Option.h"
#include "CliOptionTestUtils.h"

namespace po = boost::program_options;
namespace Utils = CliOptionTestUtils;

TEST(ParserTest, StoreAndNotify) { // NOLINT(cert-err58-cpp)
    Utils::makeCmd({"program", "-a", "1"}, [](int argc, const char **argv) {
        auto desc = po::options_description{};
        auto vm = po::variables_map{};
        auto option = CliOption::OptionBuilder<int>(desc).shortName('a').required().build();
        po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc);

        CliOption::Parser::storeAndNotify(parse.run(), vm);

        option.run(vm);
        ASSERT_EQ(option.getValue(), 1);
    });
}