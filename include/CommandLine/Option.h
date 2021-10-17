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
#include "OptionBase.h"

namespace CommandLine {

    /**
     * Represents a command line option.
     * @tparam T _value of command line option
     */
    template<typename T>
    class Option : public OptionBase<T> {
    public:
        /**
         * Create from builder.
         */
        explicit Option(OptionBuilder<T> builder);
    };

    template<typename T>
    Option<T>::Option(OptionBuilder<T> builder) : OptionBase<T>(builder, this->createTypedValue(this->isRequired(), this->getDefaultValue(), this->getImplicitValue(), "")) {
        this->checkInvariants();
    }
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTION_H
