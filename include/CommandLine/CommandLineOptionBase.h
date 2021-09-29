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

#ifndef EVGET_COMMANDLINEOPTIONBASE_H
#define EVGET_COMMANDLINEOPTIONBASE_H

#include <string>
#include <boost/program_options.hpp>
#include <optional>
#include "CommandLineOptionBuilder.h"

namespace po = boost::program_options;

/**
 * Base class for CommandLineOption that represents non-template behaviour.
 * @tparam T value of command line option
 */
template<typename T>
class CommandLineOptionBase {
public:
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
     * Set the value.
     */
    void setValue(T value);

    /**
     * Check for option conditions after reading command line arguments.
     * This must be called to notify the command line option that options
     * have been read.
     *
     * @throws InvalidCommandLineOption if conditions are not satisfied
     */
    virtual void afterRead(po::variables_map &vm);

    /**
     * Parse value component.
     */
    virtual void parseValue(po::variables_map &vm);

protected:
    /**
     * Create from builder.
     */
    explicit CommandLineOptionBase(CommandLineOptionBuilder<T> builder);

    /**
     * Get the program options description.
     */
    [[nodiscard]] boost::program_options::options_description& getOptionsDesc() const;

    /**
     * Get if value is required.
     */
    [[nodiscard]] bool isRequired() const;

    /**
     * Get conflicting options.
     */
    [[nodiscard]] const std::vector<std::string>& getConflictsWith() const;

    /**
     * If implicit value is specified from the builder.
     */
    [[nodiscard]] bool isImplicit() const;

    /**
     * If value has a default, specified from the builder.
     */
    [[nodiscard]] bool isDefaulted() const;

private:
    std::string shortName;
    std::string longName;
    std::string description;

    bool hasDefault;
    bool required;
    std::vector<std::string> conflictsWith;
    std::optional<T> implicitValue;
    std::optional<typename CommandLineOptionBuilder<T>::PerformAction> action;

    std::optional<T> value;
    std::reference_wrapper<po::options_description> desc;
};

template<typename T>
std::string CommandLineOptionBase<T>::getShortName() const {
    return shortName;
}

template<typename T>
std::string CommandLineOptionBase<T>::getLongName() const {
    return longName;
}

template<typename T>
std::string CommandLineOptionBase<T>::getDescription() const {
    return description;
}

template<typename T>
T CommandLineOptionBase<T>::getValue() const {
    return value.value();
}

template<typename T>
CommandLineOptionBase<T>::CommandLineOptionBase(CommandLineOptionBuilder<T> builder) :
    shortName{builder._shortName},
    longName{builder._longName},
    description{builder._description},
    hasDefault{builder.value.has_value()},
    required{builder._required},
    conflictsWith{builder._conflictsWith},
    action{builder._action},
    implicitValue{builder._implicitValue},
    value{builder.value},
    desc{builder._desc} {
    if (builder._positionalDesc.has_value() && builder._positionalAmount.has_value()) {
        builder._positionalDesc->get().add(
                (getShortName() + "," + getLongName()).c_str(),
                *builder._positionalAmount
        );
    }
}

template<typename T>
void CommandLineOptionBase<T>::afterRead(po::variables_map& vm) {
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

    parseValue(vm);

    // Should not happen.
    if (!value.has_value()) {
        throw UnsupportedOperationException("CommandLineOption does not have a value when it should.");
    }

    // Perform action.
    if (action.has_value()) {
        (*action)(*value);
    }
}

template<typename T>
void CommandLineOptionBase<T>::parseValue(po::variables_map& vm) {
    if (vm.count(getShortName()) && !vm[getShortName()].empty()) {
        value = vm[getShortName()].template as<T>();
    }
}

template<typename T>
void CommandLineOptionBase<T>::setValue(T value) {
    CommandLineOptionBase::value = value;
}

template<typename T>
boost::program_options::options_description& CommandLineOptionBase<T>::getOptionsDesc() const {
    return desc;
}

template<typename T>
bool CommandLineOptionBase<T>::isRequired() const {
    return required;
}

template<typename T>
const std::vector<std::string>& CommandLineOptionBase<T>::getConflictsWith() const {
    return conflictsWith;
}

template<typename T>
bool CommandLineOptionBase<T>::isImplicit() const {
    return implicitValue.has_value();
}

template<typename T>
bool CommandLineOptionBase<T>::isDefaulted() const {
    return hasDefault;
}

#endif //EVGET_COMMANDLINEOPTIONBASE_H
