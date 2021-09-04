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

#ifndef EVGET_INCLUDE_COMMANDLINEOPTION_H
#define EVGET_INCLUDE_COMMANDLINEOPTION_H

#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

/**
 * Represents a command line option.
 * @tparam value of command line option
 */
template<typename T>
class CommandLineOption {
public:
    class CommandLineOptionBuilder {
        void shortName(std::string shortName);
        void longName(std::string longName);
        void description(std::string description);
        void defaultValue(po::typed_value<T> defaultValue);
        void required();
        void conflictsWith(std::string name);
        CommandLineOption build();

        friend class CommandLineOption;
    };

    /**
     * Get short name.
     * @return shor
     */
    [[nodiscard]] const std::string getShortName() const;

    /**
     * Get long name.
     * @return long name
     */
    [[nodiscard]] const std::string getLongName() const;

    /**
     * Get description.
     * @return description
     */
    [[nodiscard]] const std::string getDescription() const;

    bool checkConflicts(po::variables_map &vm);

private:
    const std::string shortName;
    const std::string longName;
    const std::string description;
    const bool required;
    const std::vector<std::string> conflictsWith;
    po::typed_value<T> value;
};

#endif //EVGET_INCLUDE_COMMANDLINEOPTION_H
