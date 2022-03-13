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

#ifndef EVGET_CLIOPTION_INCLUDE_CLIOPTION_PARSERCOMPONENT_H
#define EVGET_CLIOPTION_INCLUDE_CLIOPTION_PARSERCOMPONENT_H

#include <boost/program_options.hpp>
namespace CliOption {
    namespace po = boost::program_options;

    class ParserComponent {
    public:
        ParserComponent() = default;

        /**
         * Perform the actual command line parsing. Returns true if operation of the program should continue.
         *
         * @param argc from main
         * @param argv from main
         * @param vm variables map
         */
        virtual bool parseCommandLine(int argc, const char* argv[], po::variables_map& vm) = 0;

        virtual ~ParserComponent() = 0;

    protected:
        ParserComponent &operator=(const ParserComponent &) = default;
        ParserComponent &operator=(ParserComponent &&) = default;
        ParserComponent(const ParserComponent &) = default;
        ParserComponent(ParserComponent &&) = default;

    private:
        std::map<std::string, std::unique_ptr<CliOption::AbstractOption>> options{};
    };
}

#endif //EVGET_CLIOPTION_INCLUDE_CLIOPTION_PARSERCOMPONENT_H
