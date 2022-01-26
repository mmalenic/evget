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

#ifndef EVGET_OPTIONVALIDATED_H
#define EVGET_OPTIONVALIDATED_H

#include "AbstractOption.h"

namespace CommandLine {

    /**
     * A command line option with custom validation.
     * @tparam T value type.
     */
    template<typename T>
    class OptionValidated : public AbstractOption<T> {
    public:
        /**
         * Create from builder.
         */
        explicit OptionValidated(OptionBuilder<T> builder,
                                 typename OptionBuilder<T>::Validator validator);

        /**
         * Create from builder.
         */
        explicit OptionValidated(OptionBuilder<T> builder,
                                 typename OptionBuilder<T>::Validator validator, const std::string& representation);

        void parseValue(po::variables_map &vm) override;

    private:
        typename OptionBuilder<T>::Validator validator;

        void setOptionDesc(auto&& createRepresentation);
    };


    template<typename T>
    OptionValidated<T>::OptionValidated(OptionBuilder<T> builder,
                                        typename OptionBuilder<T>::Validator validator)
            : AbstractOption<T>(builder), validator{validator} {
        this->checkInvariants();
        setOptionDesc([](const T& value){
            std::ostringstream representation{};
            representation << value;
            return representation.str();
        });
    }

    template<typename T>
    OptionValidated<T>::OptionValidated(OptionBuilder<T> builder,
                                        typename OptionBuilder<T>::Validator validator, const std::string& representation)
            : AbstractOption<T>(builder), validator{validator} {
        this->checkInvariants();
        setOptionDesc([&representation](const T& _){ return representation; });
    }

    template<typename T>
    void OptionValidated<T>::parseValue(po::variables_map &vm) {
        std::optional<std::string> value = this->template getValueFromVm<std::string>(vm);
        if (value.has_value() && !value->empty()) {
            std::optional<T> validatedValue = validator(*value);
            if (validatedValue.has_value()) {
                this->setValue(*validatedValue);
            } else {
                throw InvalidCommandLineOption(
                        fmt::format("Could not parse {} _defaultValue, incorrect format", this->getName()));
            }
        }
    }

    template<typename T>
    void OptionValidated<T>::setOptionDesc(auto&& createRepresentation) {
        if (this->getDefaultValue().has_value()) {
            this->template addOptionToDesc<std::string>(this->isRequired(), this->isMultitoken(), "", std::nullopt, createRepresentation(*this->getDefaultValue()));
        } else if (this->getImplicitValue().has_value()) {
            this->template addOptionToDesc<std::string>(this->isRequired(), this->isMultitoken(), std::nullopt, "", createRepresentation(*this->getImplicitValue()));
        } else if (this->getDefaultValue().has_value() && this->getImplicitValue().has_value()) {
            this->template addOptionToDesc<std::string>(this->isRequired(), this->isMultitoken(), "", "", createRepresentation(*this->getDefaultValue()));
        } else {
            this->template addOptionToDesc<std::string>(this->isRequired(), this->isMultitoken());
        }
    }
}
#endif //EVGET_OPTIONVALIDATED_H
