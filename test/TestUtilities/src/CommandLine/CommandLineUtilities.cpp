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

#include <CommandLine/CommandLineUtilities.h>

void TestUtilities::CommandLineUtilities::makeCmd(std::initializer_list<const char*> args, auto&& createCmd) {
    std::vector<const char*> vector{ args };
    vector.push_back(nullptr);
    const char** argv = vector.data();
    int argc = static_cast<int>(vector.size() - 1);
    createCmd(argc, argv);
}

void TestUtilities::CommandLineUtilities::assertExit(CommandLine& cmd) {
	ASSERT_EXIT(cmd.readArgs(), testing::ExitedWithCode(EXIT_SUCCESS), "");
}
