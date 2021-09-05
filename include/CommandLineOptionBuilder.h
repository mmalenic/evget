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

#ifndef EVGET_INCLUDE_COMMANDLINEOPTIONBUILDER_H
#define EVGET_INCLUDE_COMMANDLINEOPTIONBUILDER_H

#include <string>
#include <boost/program_options.hpp>
#include "CommandLineOption.h"

namespace po = boost::program_options;

/**
 * Command line option builder.
 * @tparam T value of option
 */
template <typename T>
class CommandLineOptionBuilder {
public:
    friend class CommandLineOption;

    /**
     * Create CommandLineOptionBuilder.
     */
    CommandLineOptionBuilder();

    /**
     * Set short name, representing one dash option.
     */
    CommandLineOptionBuilder& shortName(std::string shortName);

    /**
     * Set long name, representing two dash option.
     */
    CommandLineOptionBuilder& longName(std::string longName);

    /**
     * Set description.
     */
    CommandLineOptionBuilder& description(std::string description);

    /**
     * Set default value, if the option is not present.
     */
    CommandLineOptionBuilder& defaultValue(po::typed_value<T> defaultValue);

    /**
     * State as required option.
     */
    CommandLineOptionBuilder& required();

    /**
     * State that the option conflicts with another.
     * @param name name of conflicting option
     */
    CommandLineOptionBuilder& conflictsWith(std::string name);

    /**
     * Build CommandLineOption.
     */
    CommandLineOption build();

private:
    const std::string _shortName;
    const std::string _longName;
    const std::string _description;
    const bool _required;
    const std::vector<std::string> _conflictsWith;
    std::optional<po::typed_value<T>> value;
};

#endif //EVGET_INCLUDE_COMMANDLINEOPTIONBUILDER_H
