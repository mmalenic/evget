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
#include <utility>
#include "CommandLineOption.h"
#include "UnsupportedOperationException.h"

namespace po = boost::program_options;

/**
 * Command line option builder.
 * @tparam T value of option
 */
template <typename T>
class CommandLineOptionBuilder {
public:
    friend class CommandLineOption<T>;

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
    CommandLineOptionBuilder& defaultValue(T defaultValue);

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
     * Set the validation function.
     */
    CommandLineOptionBuilder& validator(Validator validator);

    /**
     * Build CommandLineOption.
     */
    CommandLineOption<T> build();

private:
    std::string _shortName;
    std::string _longName;
    std::string _description;
    bool _required;
    std::vector<std::string> _conflictsWith;
    std::optional<Validator> _validator;
    std::optional<T> value;
};

template <typename T>
CommandLineOptionBuilder<T>::CommandLineOptionBuilder() :
    _shortName{},
    _longName{},
    _description{},
    _required{false},
    _conflictsWith{},
    _validator{},
    value{} {
}

template <typename T>
CommandLineOptionBuilder<T>& CommandLineOptionBuilder<T>::shortName(std::string shortName) {
    _shortName = std::move(shortName);
    return *this;
}

template <typename T>
CommandLineOptionBuilder<T>& CommandLineOptionBuilder<T>::longName(std::string longName) {
    _longName = std::move(longName);
    return *this;
}

template <typename T>
CommandLineOptionBuilder<T>& CommandLineOptionBuilder<T>::description(std::string description) {
    _description = std::move(description);
    return *this;
}

template <typename T>
CommandLineOptionBuilder<T>& CommandLineOptionBuilder<T>::required() {
    _required = true;
    return *this;
}

template <typename T>
CommandLineOptionBuilder<T>& CommandLineOptionBuilder<T>::conflictsWith(std::string name) {
    _conflictsWith.emplace_back(name);
    return *this;
}

template <typename T>
CommandLineOptionBuilder<T>& CommandLineOptionBuilder<T>::defaultValue(T defaultValue) {
    value = defaultValue;
    return *this;
}

template <typename T>
CommandLineOptionBuilder<T>& CommandLineOptionBuilder<T>::validator(Validator validator) {
    _validator = validator;
    return *this;
}

template <typename T>
CommandLineOption<T> CommandLineOptionBuilder<T>::build() {
    if (!value.has_value() && !_required) {
        throw UnsupportedOperationException{"Value must at least be required, or have a default specified."};
    }
    return CommandLineOption(this);
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTIONBUILDER_H
