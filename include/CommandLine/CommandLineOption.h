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
#include <optional>
#include <fmt/core.h>
#include "InvalidCommandLineOption.h"
#include "CommandLineOptionBase.h"

/**
 * Represents a command line option.
 * @tparam T value of command line option
 */
template<typename T>
class CommandLineOption : public CommandLineOptionBase<T> {
public:
    /**
     * Create from builder.
     */
    explicit CommandLineOption(CommandLineOptionBuilder<T> builder);

    void afterRead(po::variables_map& vm) override;
};

template<typename T>
CommandLineOption<T>::CommandLineOption(CommandLineOptionBuilder<T> builder) : CommandLineOptionBase<T>(builder) {
    this->getDesc().add_options()(
       (this->getShortName() + "," + this->getLongName()).c_str(),
       po::value<T>(),
       this->getDescription().c_str()
    );
}

template<typename T>
void CommandLineOption<T>::afterRead(po::variables_map &vm) {
    if (!this->isDefaulted() && !this->isRequired()) {
        throw UnsupportedOperationException{"Value must at least be required, or have a default specified."};
    }
    CommandLineOptionBase<T>::afterRead(vm);
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTION_H
