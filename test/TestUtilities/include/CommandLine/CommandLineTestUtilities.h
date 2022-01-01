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

    /**
     * Set up and tear down command line tests.
     */
    class CommandLineTest : public testing::Test {
    protected:
        /**
         * Change to a temporary directory.
         */
        void SetUp() override;

        /**
         * Remove config file associated with running parser.
         */
        void TearDown() override;
    };

    /**
     * Store and notify vm and option.
     */
    template<typename T>
    void storeAndNotifyOption(T& option, po::command_line_parser& parse, po::variables_map& vm) {
        Cmd::Parser::storeAndNotify(parse.run(), vm);
        option.run(vm);
    }

    /**
     * Store and notify vm and option.
     */
    template<typename T>
    void storeAndNotifyOption(T& option, po::command_line_parser& parse, po::variables_map&& vm) {
        storeAndNotifyOption(option, parse, vm);
    }

    /**
     * Store and notify vm and option.
     */
    template<typename T>
    void storeAndNotifyOption(T& option, po::options_description& desc, int argc, const char** argv) {
        po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc);
        storeAndNotifyOption(option, parse, {});
    }

    /**
     * Store and notify vm and options.
     */
    template<typename T>
    void storeAndNotifyOption(std::vector<T>& options, po::command_line_parser& parse, po::variables_map& vm) {
        Cmd::Parser::storeAndNotify(parse.run(), vm);
        for (T& option : options) {
            option.run(vm);
        }
    }

    /**
     * Store and notify vm and options.
     */
    template<typename T>
    void storeAndNotifyOption(std::vector<T>& options, po::command_line_parser& parse, po::variables_map&& vm) {
        storeAndNotifyOption(options, parse, vm);
    }

    /**
     * Store and notify vm and options.
     */
    template<typename T>
    void storeAndNotifyOption(std::vector<T>& options, po::options_description& desc, int argc, const char** argv) {
        po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc);
        storeAndNotifyOption(options, parse, {});
    }

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
            throw std::overflow_error{"Number of args is larger than the maximum int value."};
        }
        int argc = static_cast<int>(vector.size() - 1);

        createCmd(argc, argv);
    }


    /**
     * Perform action with option.
     */
    void cmdWithOption(std::initializer_list<const char*> args, po::options_description& desc, auto&& option, auto&& assertCmd) {
        po::variables_map vm{};

        makeCmd(args, [&desc, &vm, &assertCmd, &option](int argc, const char** argv) {
            po::command_line_parser parse = po::command_line_parser(argc, argv).options(desc);
            assertCmd(vm, option, parse);
        });
    }

    /**
     * Assert on an option.
     */
    void withOption(std::initializer_list<const char*> args, auto&& createOption, auto&& assertCmd) {
        po::options_description desc{};
        cmdWithOption(args, desc, createOption(desc), assertCmd);
    }

    /**
     * Assert on multiple options.
     */
    template<typename T>
    void withOptionMulti(std::initializer_list<const char*> args, std::vector<Cmd::OptionBuilder<T>> builders, auto&& createOption, auto&& assertCmd) {
        for (auto builder : builders) {
            po::options_description desc{};
            cmdWithOption(args, createOption(desc, builder), assertCmd);
        }
    }

    template<typename T>
    std::vector<Cmd::OptionBuilder<T>> generateNameCombinations(Cmd::OptionBuilder<T> builder) {
        return std::vector{
            builder.shortName('n'),
            builder.longName("name"),
            builder.description("description"),
            builder.shortName('n').longName("name"),
            builder.shortName('n').description("description"),
            builder.longName("name").description("description"),
            builder.shortName('n').longName("name").description("description")
        };
    }
}

#endif //EVGET_TEST_INCLUDE_MOCKCOMMANDLINE_H