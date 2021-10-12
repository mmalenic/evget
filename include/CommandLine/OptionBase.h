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

#ifndef EVGET_OPTIONBASE_H
#define EVGET_OPTIONBASE_H

#include <string>
#include <boost/program_options.hpp>
#include <optional>
#include "OptionBuilder.h"

namespace CommandLine {

    namespace po = boost::program_options;

    /**
     * Base class for Option that represents non-template behaviour.
     * @tparam T _value of command line option
     */
    template<typename T>
    class OptionBase {
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
         * Get _value.
         */
        [[nodiscard]] T getValue() const;

        /**
         * Set the newValue.
         */
        void setValue(T newValue);

        /**
         * Check for option conditions after reading command line arguments.
         * This must be called to notify the command line option that options
         * have been read.
         *
         * @throws InvalidCommandLineOption if conditions are not satisfied
         */
        virtual void afterRead(po::variables_map &vm);

        /**
         * Parse _value component.
         */
        virtual void parseValue(po::variables_map &vm);

    protected:
        /**
         * Create from builder.
         */
        explicit OptionBase(OptionBuilder<T> builder);

        /**
         * Get the program options description.
         */
        [[nodiscard]] boost::program_options::options_description &getOptionsDesc() const;

        /**
         * Get if _value is required.
         */
        [[nodiscard]] bool isRequired() const;

        /**
         * Get conflicting options.
         */
        [[nodiscard]] const std::vector<std::string> &getConflicting() const;

        /**
         * Get the optional default value.
         */
        const std::optional<T> &getDefaultValue() const;

        /**
         * Get the optional implicit value.
         */
        const std::optional<T> &getImplicitValue() const;

        /**
         * Checks whether the option is at least, default or required. Throws
         * UnsupportedOperationException if not.
         */
        void checkInvariants();

    private:
        std::string shortName;
        std::string longName;
        std::string description;

        std::optional<T> defaultValue;
        bool required;
        std::vector<std::string> conflictsWith;
        std::optional<T> implicitValue;

        std::optional<T> _value;
        std::reference_wrapper<po::options_description> desc;
    };

    template<typename T>
    std::string OptionBase<T>::getShortName() const {
        return shortName;
    }

    template<typename T>
    std::string OptionBase<T>::getLongName() const {
        return longName;
    }

    template<typename T>
    std::string OptionBase<T>::getDescription() const {
        return description;
    }

    template<typename T>
    T OptionBase<T>::getValue() const {
        return _value.value();
    }

    template<typename T>
    OptionBase<T>::OptionBase(OptionBuilder<T> builder) :
            shortName{builder._shortName},
            longName{builder._longName},
            description{builder._description},
            defaultValue{builder.value},
            required{builder._required},
            conflictsWith{builder._conflictsWith},
            implicitValue{builder._implicitValue},
            _value{builder.value},
            desc{builder._desc} {
        if (builder._positionalDesc.has_value() && builder._positionalAmount.has_value()) {
            builder._positionalDesc->get().add(
                    (getShortName() + "," + getLongName()).c_str(),
                    *builder._positionalAmount
            );
        }
    }

    template<typename T>
    void OptionBase<T>::afterRead(po::variables_map &vm) {
        // Check required.
        if (!vm.count(getShortName()) && required) {
            throw InvalidCommandLineOption(
                    fmt::format("{} is a required option but it was not specified.", getLongName()));
        }

        // Check implicit _value.
        if (vm.count(getShortName()) && vm[getShortName()].empty()) {
            if (implicitValue.has_value()) {
                _value = implicitValue;
            } else {
                throw InvalidCommandLineOption(fmt::format("If specified, {} must have a _value", getLongName()));
            }
        }

        // Check conflicts
        for (const auto &maybeConflict: conflictsWith) {
            if (vm.count(maybeConflict)) {
                throw InvalidCommandLineOption(
                        fmt::format("Conflicting options {}, and {} specified", getLongName(), maybeConflict));
            }
        }

        parseValue(vm);

        // Should not happen.
        if (!_value.has_value()) {
            throw UnsupportedOperationException("Option does not have a _value when it should.");
        }
    }

    template<typename T>
    void OptionBase<T>::parseValue(po::variables_map &vm) {
        if (vm.count(getShortName()) && !vm[getShortName()].empty()) {
            _value = vm[getShortName()].template as<T>();
        }
    }

    template<typename T>
    void OptionBase<T>::setValue(T value) {
        OptionBase::_value = value;
    }

    template<typename T>
    boost::program_options::options_description &OptionBase<T>::getOptionsDesc() const {
        return desc;
    }

    template<typename T>
    bool OptionBase<T>::isRequired() const {
        return required;
    }

    template<typename T>
    const std::vector<std::string> &OptionBase<T>::getConflicting() const {
        return conflictsWith;
    }

    template<typename T>
    const std::optional<T> &OptionBase<T>::getDefaultValue() const {
        return defaultValue;
    }

    template<typename T>
    const std::optional<T> &OptionBase<T>::getImplicitValue() const {
        return implicitValue;
    }

    template<typename T>
    void OptionBase<T>::checkInvariants() {
        if (!this->getDefaultValue().has_value() && !this->isRequired()) {
            throw UnsupportedOperationException{
                    "Value must at least be required, implicit, or have a default specified."};
        }
    }
}

#endif //EVGET_OPTIONBASE_H
