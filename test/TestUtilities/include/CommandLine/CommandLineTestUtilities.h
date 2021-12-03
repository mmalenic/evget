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

#ifndef EVGET_TEST_INCLUDE_MOCKCOMMANDLINE_H
#define EVGET_TEST_INCLUDE_MOCKCOMMANDLINE_H

/**
 * Parser test utilities.
 */

#include <boost/program_options.hpp>
#include <CommandLine/OptionBase.h>
#include <CommandLine/Parser.h>
#include <any>
#include <gtest/gtest.h>
#include <filesystem>

namespace TestUtilities::CommandLineTestUtilities {

    namespace po = boost::program_options;
    namespace Cmd = CommandLine;
    namespace fs = std::filesystem;

    class CommandLineTest : public testing::Test {
    protected:
        /**
         * Removes config file associated with running parser.
         */
        void TearDown() override {
            Cmd::Parser parser{""};
            fs::path configFile = parser.getConfigFile();

            if (fs::exists(configFile)) {
                fs::remove(configFile);
            }
        }
    };

    /**
     * Make the command line object.
     * @param args args to use
     * @param create_cmd function to create the object
     */
    void makeCmd(std::initializer_list<const char*> args, auto&& createCmd) {
		std::vector<const char *> vector{args};
		vector.push_back(nullptr);
		const char **argv = vector.data();

        if (vector.size() - 1 > std::numeric_limits<int>::max()) {
            throw std::overflow_error{"Number of args is larger than the maximum int _defaultValue."};
        }
		int argc = static_cast<int>(vector.size() - 1);

		createCmd(argc, argv);
    }

    /**
     * Assert on an option.
     * @param args program args
     * @param createOption create option object
     * @param assertCmd assert with option
     */
    void assertOnCmd(std::initializer_list<const char*> args, auto&& createOption, auto&& assertCmd) {
        po::options_description desc{};
        po::variables_map vm{};
        auto option = createOption(desc);

        TestUtilities::CommandLineTestUtilities::makeCmd(args, [&desc, &vm, &assertCmd, &option](int argc, const char** argv) {
            po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc);
            assertCmd(vm, option, parse);
        });
    }

    /**
     * Store and notify vm and option.
     */
    template<typename T>
    void storeAndNotifyOption(Cmd::OptionBase<T>& option, po::command_line_parser& parse, po::variables_map& vm) {
        store(parse.run(), vm);
        notify(vm);
        option.run(vm);
    }
}

#endif //EVGET_TEST_INCLUDE_MOCKCOMMANDLINE_H