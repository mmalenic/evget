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

namespace po = boost::program_options;

/**
 * A Validator function takes a string as input and returns the type if valid.
 * @tparam Validate validation function
 * @tparam T return type
 */
template<typename Validate, typename T>
concept Validator = std::regular_invocable<Validate, std::string> &&
    std::convertible_to<std::invoke_result<Validate, std::string>, std::optional<T>>;


template <typename T, Validator<T> Validate>
class CommandLineOption;

/**
 * Command line option builder.
 * @tparam T value of option
 */
template <typename T, Validator<T> Validate>
class CommandLineOptionBuilder {
public:
    friend class CommandLineOption<T, Validate>;

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
    CommandLineOptionBuilder& validator(Validate&& validate);

    /**
     * Build CommandLineOption.
     */
    CommandLineOption<T, Validate> build();

private:
    std::string _shortName;
    std::string _longName;
    std::string _description;
    bool _required;
    std::vector<std::string> _conflictsWith;
    Validate&& _validate;
    std::optional<T> value;
};

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>::CommandLineOptionBuilder() :
    _shortName{},
    _longName{},
    _description{},
    _required{false},
    _conflictsWith{},
    _validate{[](const std::string& _) { return std::nullopt; }},
    value{} {
}

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>& CommandLineOptionBuilder<T, Validate>::shortName(std::string shortName) {
    _shortName = std::move(shortName);
    return *this;
}

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>& CommandLineOptionBuilder<T, Validate>::longName(std::string longName) {
    _longName = std::move(longName);
    return *this;
}

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>& CommandLineOptionBuilder<T, Validate>::description(std::string description) {
    _description = std::move(description);
    return *this;
}

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>& CommandLineOptionBuilder<T, Validate>::required() {
    _required = true;
    return *this;
}

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>& CommandLineOptionBuilder<T, Validate>::conflictsWith(std::string name) {
    _conflictsWith.emplace_back(name);
    return *this;
}

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>& CommandLineOptionBuilder<T, Validate>::defaultValue(T defaultValue) {
    value = defaultValue;
    return *this;
}

template <typename T, Validator<T> Validate>
CommandLineOptionBuilder<T, Validate>& CommandLineOptionBuilder<T, Validate>::validator(Validate&& validate) {
    _validate = validate;
    return *this;
}

template <typename T, Validator<T> Validate>
CommandLineOption<T, Validate> CommandLineOptionBuilder<T, Validate>::build() {
    return CommandLineOption(this);
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTIONBUILDER_H
