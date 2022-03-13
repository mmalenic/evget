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
#include "evget/UnsupportedOperationException.h"

namespace CliOption {

    namespace po = boost::program_options;

    // Forward declare required classes.
    template<typename T>
    class AbstractOption;

    template<typename T>
    class Option;

    class OptionFlag;

    template<typename T>
    class OptionValidated;

    template<typename U>
    concept OStreamable = requires(std::ostream &os, U value) {
        os << value;
    };

    /**
     * Command line option builder.
     * @tparam T _defaultValue of option
     */
    template<typename T>
    class OptionBuilder {
    public:
        /**
         * Validator is a function that takes a string and returns an optional _value.
         */
        using Validator = std::function<std::optional<T>(std::string)>;

        /**
         * Enable if bool, returning R.
         */
        template<typename R, typename U>
        using EnableIfBool = std::enable_if_t<sizeof(U) && (std::is_same<T, bool>::value), R>;

        /**
         * Enable if OStreamable, returning R.
         */
        template<typename R, typename U>
        using EnableIfOStreamable = std::enable_if_t<sizeof(U) && OStreamable<T>, R>;

        /**
         * Enable if not OStreamable, returning R.
         */
        template<typename R, typename U>
        using EnableIfNotOStreamable = std::enable_if_t<sizeof(U) && !OStreamable<T>, R>;

        friend class AbstractOption<T>;

        /**
         * Create OptionBuilder.
         */
        explicit OptionBuilder(po::options_description &desc);

        /**
         * Set short name, representing one dash option.
         */
        OptionBuilder &shortName(char shortName);

        /**
         * Set long name, representing two dash option.
         */
        OptionBuilder &longName(std::string longName);

        /**
         * Set description.
         */
        OptionBuilder &description(std::string description);

        /**
         * Set description.
         */
        OptionBuilder &representation(std::string representation);

        /**
         * Set default value, when the option is not present.
         */
        OptionBuilder& defaultValue(T defaultValue);

        /**
         * Set implicit value, when the option is specified without a value.
         */
        OptionBuilder& implicitValue(T implicitValue);

        /**
         * Make this option a positional _defaultValue, that takes the specified amount of values. A long name is
         * required for a positional option.
         */
        OptionBuilder &positional(int amount, po::positional_options_description &positionalDesc);

        /**
         * State as a required option.
         */
        OptionBuilder &required();

        /**
         * Make option accept multiple values.
         */
        OptionBuilder &multitoken();

        /**
         * State that the option conflicts with another.
         * @param name name of conflicting option
         */
        OptionBuilder &conflictsWith(const std::string &name);

        /**
         * State that the option conflicts with another.
         */
        OptionBuilder &conflictsWith(std::initializer_list<std::string> names);

        /**
         * Either this option or the one named should be present.
         */
        OptionBuilder &atLeast(const std::string &name);

        /**
         * Either this option or the ones named should be present.
         */
        OptionBuilder &atLeast(std::initializer_list<std::string> names);

        /**
         * Either this option or the one named should be present, except the options specified.
         */
        OptionBuilder &atLeast(const std::string &name, std::initializer_list<std::string> except);

        /**
         * At least one of the options named should be present, including this option, except the options specified.
         */
        OptionBuilder &atLeast(std::initializer_list<std::string> names, std::initializer_list<std::string> except);

        /**
         * Build flag option.
         */
        template<typename U = T>
        EnableIfBool<OptionFlag, U> buildFlag();

        /**
         * Build Option.
         */
        template<typename U = T>
        EnableIfOStreamable<Option<T>, U> build();

        /**
         * Build Option. Note if no representation for default or implicit values is specified, an
         * empty string is used as the representation.
         */
        template<typename U = T>
        EnableIfNotOStreamable<Option<T>, U> build();

        /**
         * Build Option with validator.
         */
        template<typename U = T>
        EnableIfOStreamable<OptionValidated<T>, U> build(Validator validator);

        /**
         * Build Option with validator.
         */
        template<typename U = T>
        EnableIfNotOStreamable<OptionValidated<T>, U> build(Validator validator);

    private:
        std::reference_wrapper<po::options_description> _desc;
        std::optional<std::reference_wrapper<po::positional_options_description>> _positionalDesc;
        std::string _shortName;
        std::string _longName;
        std::string _description;
        bool _required;
        bool _multitoken;
        std::vector<std::string> _conflictsWith;
        std::vector<std::string> _atLeast;
        std::vector<std::string> _exceptOption;
        std::optional<T> _implicitValue;
        std::optional<int> _positionalAmount;
        std::string _representation;
        bool _representationOptionSet;
        std::optional<T> _defaultValue;
    };

    template<typename T>
    OptionBuilder<T>::OptionBuilder(po::options_description &desc) :
            _desc{desc},
            _positionalDesc{std::nullopt},
            _shortName{},
            _longName{},
            _description{},
            _required{false},
            _multitoken{false},
            _conflictsWith{},
            _atLeast{},
            _implicitValue{std::nullopt},
            _positionalAmount{std::nullopt},
            _representation{},
            _representationOptionSet{false},
            _defaultValue{std::nullopt} {
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::shortName(char shortName) {
        _shortName = shortName;
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::longName(std::string longName) {
        _longName = std::move(longName);
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::description(std::string description) {
        _description = std::move(description);
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::representation(std::string representation) {
        _representation = std::move(representation);
        _representationOptionSet = true;
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::required() {
        _required = true;
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::multitoken() {
        _multitoken = true;
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::conflictsWith(const std::string &name) {
        _conflictsWith.emplace_back(name);
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::conflictsWith(std::initializer_list<std::string> names) {
        _conflictsWith.insert(_conflictsWith.end(), names);
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::atLeast(const std::string &name) {
        _atLeast.emplace_back(name);
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::atLeast(std::initializer_list<std::string> names) {
        _atLeast.insert(_atLeast.end(), names);
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::atLeast(const std::string &name, std::initializer_list<std::string> except) {
        _atLeast.emplace_back(name);
        _exceptOption.insert(_exceptOption.end(), except);
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::atLeast(std::initializer_list<std::string> names, std::initializer_list<std::string> except) {
        _atLeast.insert(_atLeast.end(), names);
        _exceptOption.insert(_exceptOption.end(), except);
        return *this;
    }

    template<typename T>
    OptionBuilder<T>& OptionBuilder<T>::defaultValue(T defaultValue) {
        _defaultValue = defaultValue;
        return *this;
    }

    template<typename T>
    OptionBuilder<T>& OptionBuilder<T>::implicitValue(T implicitValue) {
        _implicitValue = implicitValue;
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &
    OptionBuilder<T>::positional(int amount, po::positional_options_description &positionalDesc) {
        _positionalAmount = amount;
        _positionalDesc = positionalDesc;
        return *this;
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template EnableIfOStreamable<Option<T>, U>
    OptionBuilder<T>::build() {
        if (_representationOptionSet) {
            return Option(*this, _representation);
        }
        return Option(*this);
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template EnableIfNotOStreamable<Option<T>, U>
    OptionBuilder<T>::build() {
        return Option(*this, _representation);
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template EnableIfBool<OptionFlag, U> OptionBuilder<T>::buildFlag() {
        return OptionFlag(*this);
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template EnableIfOStreamable<OptionValidated<T>, U>
    OptionBuilder<T>::build(Validator validator) {
        if (_representationOptionSet) {
            return OptionValidated(*this, validator, _representation);
        }
        return OptionValidated(*this, validator);
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template EnableIfNotOStreamable<OptionValidated<T>, U>
    OptionBuilder<T>::build(Validator validator) {
        return OptionValidated(*this, validator, _representation);
    }
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTIONBUILDER_H
