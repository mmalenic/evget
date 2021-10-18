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

#ifndef EVGET_OPTIONVALIDATOR_H
#define EVGET_OPTIONVALIDATOR_H

#include "OptionBase.h"

namespace CommandLine {

    /**
     * A command line option with custom validation.
     * @tparam T _value type.
     */
    template<typename T>
    class OptionValidator : public OptionBase<T> {
    public:
        /**
         * Create from builder.
         */
        explicit OptionValidator(OptionBuilder<T> builder,
                                 typename OptionBuilder<T>::Validator validator);

        void parseValue(po::variables_map &vm) override;

    private:
        typename OptionBuilder<T>::Validator validator;
    };


    template<typename T>
    OptionValidator<T>::OptionValidator(OptionBuilder<T> builder,
                                        typename OptionBuilder<T>::Validator validator)
            : OptionBase<T>(builder, this->template setTypedValue<std::string>(this->isRequired(), std::optional{""}, std::optional{""}, "")), validator{validator} {
        this->checkInvariants();
    }

    template<typename T>
    void OptionValidator<T>::parseValue(po::variables_map &vm) {
        if (this->isOptionPresent(vm) && !this->isOptionEmpty(vm)) {
            std::optional<T> validatedValue = validator(*this->template getValueFromVm<std::string>(vm));
            if (validatedValue.has_value()) {
                this->setValue(*validatedValue);
            } else {
                throw InvalidCommandLineOption(
                        fmt::format("Could not parse {} value, incorrect format", this->getLongName()));
            }
        }
    }
}
#endif //EVGET_OPTIONVALIDATOR_H
