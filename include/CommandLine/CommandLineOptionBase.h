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

#ifndef EVGET_COMMANDLINEOPTIONBASE_H
#define EVGET_COMMANDLINEOPTIONBASE_H

#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

/**
 * Base class for CommandLineOption that represents non-template behaviour.
 */
class CommandLineOptionBase {
public:

    /**
     * Constructor for base option class.
     */
    CommandLineOptionBase(std::string shortName, std::string longName, std::string description);

    /**
     * Get short name.
     */
    [[nodiscard]] std::string getShortName() const;

    /**
     * Get long name.
     */
    [[nodiscard]] std::string getLongName() const;

    /**
     * Get description.
     */
    [[nodiscard]] std::string getDescription() const;

    /**
     * Check for option conditions after reading command line arguments.
     * This must be called to notify the command line option that options
     * have been read.
     *
     * @throws InvalidCommandLineOption if conditions are not satisfied
     */
    virtual void afterRead(po::variables_map &vm) = 0;

    virtual ~CommandLineOptionBase() = default;
    CommandLineOptionBase(const CommandLineOptionBase&) = default;
    CommandLineOptionBase(CommandLineOptionBase&&) noexcept = default;
    CommandLineOptionBase& operator=(const CommandLineOptionBase&) = default;
    CommandLineOptionBase& operator=(CommandLineOptionBase&&) noexcept = default;

private:
    std::string shortName;
    std::string longName;
    std::string description;
};

#endif //EVGET_COMMANDLINEOPTIONBASE_H
