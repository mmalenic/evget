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

#include "clioption/Parser.h"

CliOption::Parser::Parser(const std::string& cmdlineDesc, const std::string& configDesc, const std::string& envDesc)
    : _cmdlineDesc{cmdlineDesc}, _configDesc{configDesc}, _envDesc{envDesc} {}

void CliOption::Parser::parseConfig(std::istream& stream) {
    storeAndNotify(parse_config_file(stream, _configDesc), vm);
}

void CliOption::Parser::parseEnv(const char* prefix) {
    storeAndNotify(parse_environment(_envDesc, prefix), vm);
}

void CliOption::Parser::parseCommandLine(int argc, const char* argv[]) {
    storeAndNotify(po::command_line_parser(argc, argv).options(_cmdlineDesc).allow_unregistered().run(), vm);
}

void CliOption::Parser::storeAndNotify(
    const boost::program_options::parsed_options& parsedOptions,
    CliOption::po::variables_map& vm
) {
    po::store(parsedOptions, vm);
    po::notify(vm);
}

boost::program_options::options_description& CliOption::Parser::cmdlineDesc() {
    return _cmdlineDesc;
}

boost::program_options::options_description& CliOption::Parser::configDesc() {
    return _configDesc;
}

boost::program_options::options_description& CliOption::Parser::envDesc() {
    return _envDesc;
}

boost::program_options::options_description& CliOption::Parser::addCmdline(const po::options_description& description) {
    return _cmdlineDesc.add(description);
}

boost::program_options::options_description& CliOption::Parser::addConfig(const po::options_description& description) {
    return _configDesc.add(description);
}

boost::program_options::options_description& CliOption::Parser::addEnv(const po::options_description& description) {
    return _envDesc.add(description);
}