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
#include "CommandLineOptionBuilder.h"
#include "InvalidCommandLineOption.h"

namespace po = boost::program_options;

/**
 * Represents a command line option.
 * @tparam T value of command line option
 */
template<typename T>
class CommandLineOption {
public:
    /**
     * Create from builder.
     */
    explicit CommandLineOption(CommandLineOptionBuilder<T> builder);

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
     * Get value.
     */
    [[nodiscard]] T getValue() const;

    /**
     * Check for option conditions after reading command line arguments.
     * This must be called to notify the command line option that options
     * have been read.
     *
     * @throws InvalidCommandLineOption if conditions are not satisfied
     */
    void afterRead(po::variables_map &vm);

private:
    const std::string shortName;
    const std::string longName;
    const std::string description;
    const bool required;
    const std::vector<std::string> conflictsWith;
    const std::optional<T> implicitValue;
    const std::optional<typename CommandLineOptionBuilder<T>::Validator> validator;

    std::string stringValue;
    std::optional<T> value;
};

template<typename T>
CommandLineOption<T>::CommandLineOption(CommandLineOptionBuilder<T> builder) :
    shortName{builder._shortName},
    longName{builder._longName},
    description{builder._description},
    required{builder._requied},
    conflictsWith{builder._conflictsWith},
    validator{builder._validator},
    implicitValue{builder._implicitValue},
    value{builder.value} {
    builder._desc.add_options()(
        (shortName + "," + longName).c_str(),
        po::value<std::string>() ? validator.has_value() : po::value<T>(),
        description.c_str()
    );
    if (builder._positionalDesc.has_value() && builder._positionalAmount.has_value()) {
        builder._positionalDesc->get().add(
            (shortName + "," + longName).c_str(),
            builder._positionalAmount
        );
    }
}

template<typename T>
T CommandLineOption<T>::getValue() const {
    return value;
}

template<typename T>
std::string CommandLineOption<T>::getShortName() const {
    return shortName;
}

template<typename T>
std::string CommandLineOption<T>::getLongName() const {
    return longName;
}

template<typename T>
std::string CommandLineOption<T>::getDescription() const {
    return description;
}

template<typename T>
void CommandLineOption<T>::afterRead(po::variables_map& vm) {
    // Check required.
    if (!vm.count(shortName) && required) {
        throw InvalidCommandLineOption(fmt::format("{} is a required option but it was not specified.", longName));
    }

    // Check implicit value.
    if (vm.count(shortName) && vm[shortName].empty()) {
        if (implicitValue.has_value()) {
            value = implicitValue;
        } else {
            throw InvalidCommandLineOption(fmt::format("If specified, {} must have a value", longName));
        }
    }

    // Check conflicts
    for (const auto& maybeConflict : conflictsWith) {
        if (shortName == maybeConflict || longName == maybeConflict) {
            throw InvalidCommandLineOption(fmt::format("Conflicting options {}, and {} specified", longName, maybeConflict));
        }
    }

    // Check regular value and validator.
    if (vm.count(shortName) && !vm[shortName].empty()) {
        if (!validator.has_value()) {
            value = vm[shortName].as<T>();
        } else {
            std::optional<std::string> validatedValue = validator(vm[shortName].as<std::string>());
            if (validatedValue.has_value()) {
                value = validatedValue;
            } else {
                throw InvalidCommandLineOption(fmt::format("Could not parse {} value, incorrect format", longName));
            }
        }
    }

    // Should not happen.
    if (!value.has_value()) {
        throw UnsupportedOperationException("CommandLineOption does not have a value when it should.");
    }
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTION_H
