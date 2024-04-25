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

#ifndef EVGET_CLIOPTION_INCLUDE_CLIOPTION_PARSER_H
#define EVGET_CLIOPTION_INCLUDE_CLIOPTION_PARSER_H

#include <boost/program_options.hpp>

#include "AbstractOption.h"

namespace CliOption {
namespace po = boost::program_options;

class Parser {
public:
    Parser() = default;

    Parser(const std::string& cmdlineDesc, const std::string& configDesc, const std::string& envDesc);

    /**
     * Perform the command line parsing.
     *
     * @param argc from main
     * @param argv from main
     */
    void parseCommandLine(int argc, const char* argv[]);

    /**
     * Perform the config parsing.
     *
     * @param stream config file stream
     */
    void parseConfig(std::istream& stream);

    /**
     * Perform the environment variable parsing.
     *
     * @param prefix environment variable prefix
     */
    void parseEnv(const char* prefix);

    /**
     * Parse an option and register it with the variables map.
     *
     * @param option option to parse
     */
    template <typename T>
    void parseOption(AbstractOption<T> option);

    /**
     * Store and notify the variables map.
     */
    static void storeAndNotify(const po::parsed_options& parsedOptions, po::variables_map& vm);

private:
    po::variables_map vm{};

    po::options_description cmdlineDesc{};
    po::options_description configDesc{};
    po::options_description envDesc{};
};

template <typename T>
void Parser::parseOption(AbstractOption<T> option) {
    option.run(vm);
}

}  // namespace CliOption

#endif  // EVGET_CLIOPTION_INCLUDE_CLIOPTION_PARSER_H
