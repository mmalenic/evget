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
#include <fmt/core.h>
#include <fstream>
#include "OptionBuilder.h"
#include "InvalidCommandLineOption.h"

namespace CommandLine {

    namespace po = boost::program_options;

    /**
     * Base class for Option that represents non-template behaviour.
     * @tparam T _defaultValue of command line option
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
         * Get value.
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
         * Get the optional default value.
         */
        [[nodiscard]] virtual std::optional<T> getDefaultValue() const;

        /**
         * Get the optional implicit value.
         */
        [[nodiscard]] virtual std::optional<T> getImplicitValue() const;

        /**
         * Get the representation.
         */
        [[nodiscard]] std::string getRepresentation() const;

        /**
         * Get if _defaultValue is required.
         */
        [[nodiscard]] bool isRequired() const;

        /**
         * Check for option conditions after reading command line arguments.
         * This must be called to notify the command line option that options
         * have been read.
         *
         * @throws InvalidCommandLineOption if conditions are not satisfied
         */
        virtual void run(po::variables_map &vm);

        /**
         * Parse value component.
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
         * Get if _defaultValue is multitoken.
         */
        [[nodiscard]] bool isMultitoken() const;

        /**
         * Get conflicting options.
         */
        [[nodiscard]] const std::vector<std::string> &getConflicting() const;

        /**
         * Checks whether the option is at least, default or required. Throws
         * UnsupportedOperationException if not.
         */
        void checkInvariants();

        /**
         * Check if the _defaultValue is present in the option.
         * @return
         */
        bool isValuePresent();

        /**
         * Get the _defaultValue from the variables map.
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
         * Check if the option is present in the variables map.
         */
        bool isOptionPresent(po::variables_map &vm);

        /**
         * Add an option to the options description.
         */
        template<typename U>
        void addOptionToDesc(bool required,
                             bool multitoken,
                             std::optional<U> defaultValue,
                             std::optional<U> implicitValue,
                             const std::string& representation,
                             po::typed_value<U>* typedValue = po::value<U>());

        /**
         * Add an option to the options description.
         */
        template<typename U>
        void addOptionToDesc(bool required,
                             bool multitoken,
                             std::optional<U> defaultValue,
                             std::optional<U> implicitValue,
                             po::typed_value<U>* typedValue = po::value<U>());

        /**
         * Add an option to the options description.
         */
        template<typename U>
        void addOptionToDesc(bool required,
                             bool multitoken,
                             po::typed_value<U>* typedValue = po::value<U>());

        /**
         * Add an option to the options description.
         */
        template<typename U>
        void addOptionToDesc(po::typed_value<U>* typedValue);

    private:
        std::string shortName;
        std::string shortNameKey;
        std::string longName;
        std::string longNameKey;
        std::string description;

        std::optional<T> defaultValue;
        bool required;
        bool multitoken;
        std::vector<std::string> conflictsWith;
        std::optional<T> implicitValue;
        std::string representation;

        std::optional<T> _value;
        std::reference_wrapper<po::options_description> desc;

        /**
         * Create the typed _defaultValue using default and implicit values.
         * Use of raw pointer ensures consistency and a lack of bugs
         * with the boost program options library.
         */
        template<typename U>
        static po::typed_value<U>* setTypedValue(
                bool required,
                bool multitoken,
                po::typed_value<U>* typedValue = po::value<U>()
        );

        template<typename U>
        static po::typed_value<U>* setTypedValue(
                std::optional<U> defaultValue,
                std::optional<U> implicitValue,
                const std::string& representation,
                po::typed_value<U>* typedValue = po::value<U>()
        );

        template<typename U>
        static po::typed_value<U>* setTypedValue(
                std::optional<U> defaultValue,
                std::optional<U> implicitValue,
                po::typed_value<U>* typedValue = po::value<U>()
        );

        void addPositionalOption(OptionBuilder<T>& builder, std::string name);
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
            defaultValue{builder._defaultValue},
            required{builder._required},
            multitoken{builder._multitoken},
            conflictsWith{builder._conflictsWith},
            implicitValue{builder._implicitValue},
            representation{builder._representation},
            _value{builder._defaultValue},
            desc{builder._desc} {
        if (shortName.empty() && longName.empty()) {
            throw UnsupportedOperationException("Option should contain at least a short name, or a long name.");
        }

        if (builder._positionalDesc.has_value() && builder._positionalAmount.has_value()) {
            if (!longName.empty()) {
                addPositionalOption(builder, longNameKey);
            } else if (!shortName.empty()) {
                addPositionalOption(builder, shortNameKey);
            }
        }

        if (defaultValue.has_value()) {
            _value = defaultValue;
        }
        if (implicitValue.has_value()) {
            _value = implicitValue;
        }
    }

    template<typename T>
    void OptionBase<T>::run(po::variables_map &vm) {
        parseValue(vm);

        // Check conflicts
        for (const auto &maybeConflict: conflictsWith) {
            if (vm.count(maybeConflict) || vm.count(fmt::format("-{}", maybeConflict))) {
                throw InvalidCommandLineOption(
                        fmt::format("Conflicting options {}, and {} specified", getName(), maybeConflict));
            }
        }

        // Unsupported use of option.
        if (!_value.has_value()) {
            throw UnsupportedOperationException("Unsupported use of option.");
        }
    }

    template<typename T>
    void OptionBase<T>::parseValue(po::variables_map &vm) {
        _value = getValueFromVm<T>(vm);
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
    bool OptionBase<T>::isMultitoken() const {
        return multitoken;
    }

    template<typename T>
    const std::vector<std::string> &OptionBase<T>::getConflicting() const {
        return conflictsWith;
    }

    template<typename T>
    std::optional<T> OptionBase<T>::getDefaultValue() const {
        return defaultValue;
    }

    template<typename T>
    std::optional<T> OptionBase<T>::getImplicitValue() const {
        return implicitValue;
    }

    template<typename T>
    std::string OptionBase<T>::getRepresentation() const {
        return representation;
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
        return (vm.count(longNameKey) && vm.at(longNameKey).empty()) || (vm.count(shortNameKey) && vm.at(shortNameKey).empty());
    }

    template<typename T>
    bool OptionBase<T>::isOptionPresentAndNotEmpty(po::variables_map &vm) {
        return (vm.count(longNameKey) && !vm.at(longNameKey).empty()) || (vm.count(shortNameKey) && !vm.at(shortNameKey).empty());
    }

    template<typename T>
    std::string OptionBase<T>::getName() {
        if (!longName.empty()) {
            return longName;
        }
        return shortName;
    }

    template<typename T>
    bool OptionBase<T>::isValuePresent() {
        return _value.has_value();
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
    void OptionBase<T>::addOptionToDesc(
            bool required,
            bool multitoken,
            po::typed_value<U>* typedValue
    ) {
        OptionBase<T>::setTypedValue(required, multitoken, typedValue);
        this->addOptionToDesc(typedValue);
    }

    template<typename T>
    template<typename U>
    void OptionBase<T>::addOptionToDesc(
            bool required,
            bool multitoken,
            std::optional<U> defaultValue,
            std::optional<U> implicitValue,
            const std::string& representation,
            po::typed_value<U>* typedValue
    ) {
        OptionBase<T>::setTypedValue(required, multitoken, typedValue);
        OptionBase<T>::setTypedValue(defaultValue, implicitValue, representation, typedValue);
        this->addOptionToDesc(typedValue);
    }

    template<typename T>
    template<typename U>
    void OptionBase<T>::addOptionToDesc(
            bool required,
            bool multitoken,
            std::optional<U> defaultValue,
            std::optional<U> implicitValue,
            po::typed_value<U>* typedValue
    ) {
        OptionBase<T>::setTypedValue(required, multitoken, typedValue);
        OptionBase<T>::setTypedValue(defaultValue, implicitValue, typedValue);
        this->addOptionToDesc(typedValue);
    }

    template<typename T>
    template<typename U>
    po::typed_value<U>* OptionBase<T>::setTypedValue(
            bool required,
            bool multitoken,
            po::typed_value<U>* typedValue
            ) {
        if (required) {
            typedValue->required();
        }
        if (multitoken) {
            typedValue->multitoken();
        }
        return typedValue;
    }

    template<typename T>
    template<typename U>
    po::typed_value<U>* OptionBase<T>::setTypedValue(
            std::optional<U> defaultValue,
            std::optional<U> implicitValue,
            const std::string& representation,
            po::typed_value<U>* typedValue
    ) {
        if (defaultValue.has_value()) {
            typedValue->default_value(*defaultValue, representation);
        }
        if (implicitValue.has_value()) {
            typedValue->implicit_value(*implicitValue, representation);
        }
        return typedValue;
    }

    template<typename T>
    template<typename U>
    po::typed_value<U>* OptionBase<T>::setTypedValue(
            std::optional<U> defaultValue,
            std::optional<U> implicitValue,
            po::typed_value<U>* typedValue
    ) {
        if (defaultValue.has_value()) {
            typedValue->default_value(*defaultValue);
        }
        if (implicitValue.has_value()) {
            typedValue->implicit_value(*implicitValue);
        }
        return typedValue;
    }


    template<typename T>
    void OptionBase<T>::addPositionalOption(OptionBuilder<T>& builder, std::string name) {
        builder._positionalDesc->get().add(
                (name).c_str(),
                *builder._positionalAmount
        );
    }
}

#endif //EVGET_OPTIONBASE_H
