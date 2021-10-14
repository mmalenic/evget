#include <utility>

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

#ifndef EVGET_OPTIONFLAG_H
#define EVGET_OPTIONFLAG_H

#include "OptionBase.h"

namespace CommandLine {

    class OptionFlag : public OptionBase<bool> {
    public:
        /**
         * Create from builder.
         */
        explicit OptionFlag(OptionBuilder<bool> builder);

        void parseValue(po::variables_map &vm) override;
        void parseDefaultValue(po::variables_map &vm) override;
        void parseImplicitValue(po::variables_map &vm) override;
    };

    OptionFlag::OptionFlag(OptionBuilder<bool> builder) : OptionBase<bool>(std::move(builder), std::unique_ptr<po::typed_value<bool>>{po::bool_switch()}) {
    }

    void OptionFlag::parseValue(po::variables_map &vm) {
        this->setValue(vm.count(getShortName()));
    }

    void OptionFlag::parseDefaultValue(po::variables_map &vm) {
    }

    void OptionFlag::parseImplicitValue(po::variables_map &vm) {
    }
}


#endif //EVGET_OPTIONFLAG_H
