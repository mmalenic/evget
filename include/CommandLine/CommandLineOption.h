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
#include "CommandLineOptionBase.h"

namespace po = boost::program_options;

/**
 * Represents a command line option.
 * @tparam T value of command line option
 */
template<typename T>
class CommandLineOption : public CommandLineOptionBase {
public:
    /**
     * Create from builder.
     */
    explicit CommandLineOption(CommandLineOptionBuilder<T> builder);

    /**
     * Get value.
     */
    [[nodiscard]] T getValue() const;

    void afterRead(po::variables_map &vm) override;

private:
    const bool required;
    const std::vector<std::string> conflictsWith;
    const std::optional<T> implicitValue;
    const std::optional<typename CommandLineOptionBuilder<T>::Validator> validator;

    std::string stringValue;
    std::optional<T> value;
};

template<typename T>
CommandLineOption<T>::CommandLineOption(CommandLineOptionBuilder<T> builder) :
    CommandLineOptionBase{builder._shortName, builder._longName, builder._description},
    required{builder._required},
    conflictsWith{builder._conflictsWith},
    validator{builder._validator},
    implicitValue{builder._implicitValue},
    value{builder.value} {

    if (validator.has_value()) {
        builder._desc.add_options()(
            (getShortName() + "," + getLongName()).c_str(),
            po::value<std::string>(),
            getDescription().c_str()
        );
    } else {
        builder._desc.add_options()(
            (getShortName() + "," + getLongName()).c_str(),
            po::value<T>(),
            getDescription().c_str()
        );
    }

    if (builder._positionalDesc.has_value() && builder._positionalAmount.has_value()) {
        builder._positionalDesc->get().add(
            (getShortName() + "," + getLongName()).c_str(),
            *builder._positionalAmount
        );
    }
}

template<typename T>
T CommandLineOption<T>::getValue() const {
    return value.value();
}

template<typename T>
void CommandLineOption<T>::afterRead(po::variables_map& vm) {
    // Check required.
    if (!vm.count(getShortName()) && required) {
        throw InvalidCommandLineOption(fmt::format("{} is a required option but it was not specified.", getLongName()));
    }

    // Check implicit value.
    if (vm.count(getShortName()) && vm[getShortName()].empty()) {
        if (implicitValue.has_value()) {
            value = implicitValue;
        } else {
            throw InvalidCommandLineOption(fmt::format("If specified, {} must have a value", getLongName()));
        }
    }

    // Check conflicts
    for (const auto& maybeConflict : conflictsWith) {
        if (getShortName() == maybeConflict || getLongName() == maybeConflict) {
            throw InvalidCommandLineOption(fmt::format("Conflicting options {}, and {} specified", getLongName(), maybeConflict));
        }
    }

    // Check regular value and validator.
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

    // Should not happen.
    if (!value.has_value()) {
        throw UnsupportedOperationException("CommandLineOption does not have a value when it should.");
    }
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTION_H
