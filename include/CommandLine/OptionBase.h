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
         * Return the name of this option, which is the longName if present,
         * otherwise it is the shortName.
         */
        std::string getName();

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

        /**
         * Parse implicit value component.
         */
        virtual void parseImplicitValue(po::variables_map &vm);

        /**
         * Parse default value component.
         */
        virtual void parseDefaultValue(po::variables_map &vm);

        /**
         * Performs the after read for all the options, handling exceptions thrown by the
         * boost program options library.
         */
        static void afterReadForAll(std::initializer_list<std::reference_wrapper<OptionBase<T>>> options, po::variables_map& vm);

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

        /**
         * Get the value from the variables map.
         */
        template<typename U>
        std::optional<U> getValueFromVm(po::variables_map &vm);

        /**
         * Check if the option is present but empty.
         */
        bool isOptionPresentAndEmpty(po::variables_map &vm);

        /**
         * Check if the option is present but not empty.
         */
        bool isOptionPresentAndNotEmpty(po::variables_map &vm);

        /**
         * Check if the value is present in the variables map.
         */
        bool isOptionPresent(po::variables_map &vm);

        /**
         * Add an option to the options description.
         */
        template<typename U>
        void addOptionToDesc(po::typed_value<U>* typedValue);

        /**
         * Create the typed value using default and implicit values.
         * Use of raw pointer ensures consistency and a lack of bugs
         * with the boost program options library.
         */
        template<typename U>
        static po::typed_value<U>* setTypedValue(
                bool required,
                std::optional<U> defaultValue,
                std::optional<U> implicitValue,
                std::string representation,
                po::typed_value<U>* typedValue = po::value<U>()
                );

    private:
        std::string shortName;
        std::string shortNameKey;
        std::string longName;
        std::string longNameKey;
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
            shortNameKey{fmt::format("-{}", shortName)},
            longName{builder._longName},
            longNameKey{builder._longName},
            description{builder._description},
            defaultValue{builder.value},
            required{builder._required},
            conflictsWith{builder._conflictsWith},
            implicitValue{builder._implicitValue},
            _value{builder.value},
            desc{builder._desc} {
        if (shortName.empty() && longName.empty()) {
            throw UnsupportedOperationException("Option should contain at least a short name, or a long name.");
        }

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
        if (!isOptionPresent(vm) && required) {
            throw InvalidCommandLineOption(
                    fmt::format("{} is a required option but it was not specified.", getName()));
        }

        // Parse value.
        parseDefaultValue(vm);
        parseImplicitValue(vm);
        parseValue(vm);

        // Check conflicts
        for (const auto &maybeConflict: conflictsWith) {
            if (vm.count(maybeConflict) || vm.count(fmt::format("-{}", maybeConflict))) {
                throw InvalidCommandLineOption(
                        fmt::format("Conflicting options {}, and {} specified", getName(), maybeConflict));
            }
        }

        // Should not happen.
        if (!_value.has_value()) {
            throw UnsupportedOperationException("Option does not have a value when it should.");
        }
    }

    template<typename T>
    void OptionBase<T>::parseValue(po::variables_map &vm) {
        _value = getValueFromVm<T>(vm);
    }

    template<typename T>
    void OptionBase<T>::parseDefaultValue(po::variables_map &vm) {
        if (!isOptionPresent(vm) && defaultValue.has_value()) {
            _value = defaultValue;
        }
    }

    template<typename T>
    void OptionBase<T>::parseImplicitValue(po::variables_map &vm) {
        if (isOptionPresentAndEmpty(vm)) {
            if (implicitValue.has_value()) {
                _value = implicitValue;
            } else {
                throw InvalidCommandLineOption(fmt::format("If specified, {} must have a value", getName()));
            }
        }
    }

    template<typename T>
    void OptionBase<T>::setValue(T value) {
        _value = value;
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
            throw UnsupportedOperationException{"Value must at least be required, or have a default specified."};
        }
    }

    template<typename T>
    template<typename U>
    std::optional<U> OptionBase<T>::getValueFromVm(po::variables_map &vm) {
        if (vm.count(longNameKey) && !vm.at(longNameKey).empty()) {
            return vm[longNameKey].template as<U>();
        }
        if (vm.count(shortNameKey) && !vm.at(shortNameKey).empty()) {
            return vm[shortNameKey].template as<U>();
        }
        return std::nullopt;
    }

    template<typename T>
    bool OptionBase<T>::isOptionPresent(po::variables_map &vm) {
        return vm.count(longNameKey) || vm.count(shortNameKey);
    }

    template<typename T>
    bool OptionBase<T>::isOptionPresentAndEmpty(po::variables_map &vm) {
        return (vm.count(longNameKey) && vm.at(longNameKey).empty()) || (vm.count(shortNameKey) || vm.at(shortNameKey).empty());
    }

    template<typename T>
    bool OptionBase<T>::isOptionPresentAndNotEmpty(po::variables_map &vm) {
        return (vm.count(longNameKey) && !vm.at(longNameKey).empty()) || (vm.count(shortNameKey) || !vm.at(shortNameKey).empty());
    }

    template<typename T>
    std::string OptionBase<T>::getName() {
        if (!longName.empty()) {
            return longName;
        }
        return shortName;
    }


    template<typename T>
    template<typename U>
    void OptionBase<T>::addOptionToDesc(po::typed_value<U>* typedValue) {
        this->getOptionsDesc().add_options()(
                (this->getLongName() + "," + this->getShortName()).c_str(),
                typedValue,
                this->getDescription().c_str()
        );
    }

    template<typename T>
    template<typename U>
    po::typed_value<U>* OptionBase<T>::setTypedValue(
            bool required,
            std::optional<U> defaultValue,
            std::optional<U> implicitValue,
            std::string representation,
            po::typed_value<U>* typedValue
            ) {
        if (required) {
            typedValue->required();
        }
        if (defaultValue.has_value()) {
            typedValue->default_value(*defaultValue, representation);
        }
        if (implicitValue.has_value()) {
            typedValue->implicit_value(*implicitValue, representation);
        }
        return typedValue;
    }

    template<typename T>
    void OptionBase<T>::afterReadForAll(std::initializer_list<std::reference_wrapper<OptionBase<T>>> options, po::variables_map& vm) {
        std::exception_ptr e_ptr;
        try {
            po::notify(vm);
        } catch (po::error& error) {
            e_ptr = std::current_exception();
        }

        for (auto option : options) {
            option.afterRead(vm);
        }

        if (e_ptr) {
            std::rethrow_exception(e_ptr);
        }
    }
}

#endif //EVGET_OPTIONBASE_H
