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

#ifndef EVGET_COMMANDLINEOPTIONVALIDATOR_H
#define EVGET_COMMANDLINEOPTIONVALIDATOR_H

#include "CommandLineOptionBase.h"

/**
 * A command line option with custom validation.
 * @tparam T value type.
 */
template<typename T>
class CommandLineOptionValidator : public CommandLineOptionBase<T> {
public:
    /**
     * Create from builder.
     */
    explicit CommandLineOptionValidator(CommandLineOptionBuilder<T> builder);

    void parseValue(po::variables_map &vm) override;

private:
    std::optional<typename CommandLineOptionBuilder<T>::Validator> validator;
};


template<typename T>
CommandLineOptionValidator<T>::CommandLineOptionValidator(CommandLineOptionBuilder<T> builder) : CommandLineOptionBase<T>(builder), validator{builder._validator} {
    builder._desc.get().add_options()(
        (getShortName() + "," + getLongName()).c_str(),
        po::value<std::string>(),
        getDescription().c_str()
    );
}

template<typename T>
void CommandLineOptionValidator<T>::parseValue(po::variables_map &vm) {
    if (vm.count(getShortName()) && !vm[getShortName()].empty()) {
        if (!validator.has_value()) {
            value = vm[getShortName()].template as<T>();
        } else {
            std::optional<T> validatedValue = (*validator)(vm[getShortName()].template as<std::string>());
            if (validatedValue.has_value()) {
                value = validatedValue;
            } else {
                throw InvalidCommandLineOption(fmt::format("Could not parse {} value, incorrect format", getLongName()));
            }
        }
    }
}

#endif //EVGET_COMMANDLINEOPTIONVALIDATOR_H
