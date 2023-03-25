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

#include <boost/program_options.hpp>
#include <fmt/core.h>

#include <optional>
#include <string>

#include "AbstractOption.h"
#include "InvalidCommandLineOption.h"

namespace CliOption {

/**
 * Represents a command line option.
 * @tparam T _value of command line option
 */
template <typename T>
class Option : public AbstractOption<T> {
public:
    /**
     * Create from builder.
     */
    explicit Option(OptionBuilder<T> builder);

    /**
     * Create from builder.
     */
    Option(OptionBuilder<T> builder, const std::string& representation);
};

template <typename T>
Option<T>::Option(OptionBuilder<T> builder) : AbstractOption<T>(builder) {
    this->checkInvariants();
    this->template addOptionToDesc<T>(
        this->isRequired(),
        this->isMultitoken(),
        this->getDefaultValue(),
        this->getImplicitValue()
    );
}

template <typename T>
Option<T>::Option(OptionBuilder<T> builder, const std::string& representation) : AbstractOption<T>(builder) {
    this->checkInvariants();
    this->addOptionToDesc(
        this->isRequired(),
        this->isMultitoken(),
        this->getDefaultValue(),
        this->getImplicitValue(),
        representation
    );
}
}  // namespace CliOption

#endif  // EVGET_INCLUDE_COMMANDLINEOPTION_H
