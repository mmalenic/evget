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
#include "UnsupportedOperationException.h"

namespace CommandLine {

    namespace po = boost::program_options;

    // Forward declare required classes.
    template<typename T>
    class OptionBase;

    template<typename T>
    class Option;

    class OptionFlag;

    template<typename T>
    class OptionValidator;

    /**
     * Command line option builder.
     * @tparam T _value of option
     */
    template<typename T>
    class OptionBuilder {
    public:
        /**
         * Validator is a function that takes a string and returns an optional _value.
         */
        using Validator = std::function<std::optional<T>(std::string)>;

        /**
         * Disable if bool, returning R.
         */
        template<typename R, typename U>
        using DisableIfBool = std::enable_if_t<sizeof(U) && (!std::is_same<T, bool>::value), R>;

        /**
         * Enable if bool, returning R.
         */
        template<typename R, typename U>
        using EnableIfBool = std::enable_if_t<sizeof(U) && (std::is_same<T, bool>::value), R>;


        friend class OptionBase<T>;

        /**
         * Create OptionBuilder.
         */
        explicit OptionBuilder(po::options_description &desc);

        /**
         * Set short name, representing one dash option.
         */
        OptionBuilder &shortName(std::string shortName);

        /**
         * Set long name, representing two dash option.
         */
        OptionBuilder &longName(std::string longName);

        /**
         * Set description.
         */
        OptionBuilder &description(std::string description);

        /**
         * Set default _value, if the option is not present.
         */
        template<typename U = T>
        DisableIfBool<OptionBuilder &, U> defaultValue(T defaultValue);

        /**
         * Set implicit _value, if the option is specified without a _value.
         */
        template<typename U = T>
        DisableIfBool<OptionBuilder &, U> implicitValue(T implicitValue);

        /**
         * Make this optional a positional _value, that takes the amount of values.
         */
        OptionBuilder &positionalValue(int amount, po::positional_options_description &positionalDesc);

        /**
         * State as required option.
         */
        OptionBuilder &required();

        /**
         * State that the option conflicts with another.
         * @param name name of conflicting option
         */
        OptionBuilder &conflictsWith(const std::string &name);

        /**
         * Build flag option.
         */
        template<typename U = T>
        EnableIfBool<OptionFlag, U> buildFlag();

        /**
         * Build Option.
         */
        Option<T> build();

        /**
         * Build OptionValidator
         */
        template<typename U = T>
        DisableIfBool<OptionValidator<T>, U> build(Validator validator);

    private:
        std::reference_wrapper<po::options_description> _desc;
        std::optional<std::reference_wrapper<po::positional_options_description>> _positionalDesc;
        std::string _shortName;
        std::string _longName;
        std::string _description;
        bool _required;
        std::vector<std::string> _conflictsWith;
        std::optional<T> _implicitValue;
        std::optional<int> _positionalAmount;
        std::optional<T> value;
    };

    template<typename T>
    OptionBuilder<T>::OptionBuilder(po::options_description &desc) :
            _desc{desc},
            _positionalDesc{std::nullopt},
            _shortName{},
            _longName{},
            _description{},
            _required{false},
            _conflictsWith{},
            _implicitValue{std::nullopt},
            _positionalAmount{std::nullopt},
            value{std::nullopt} {
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::shortName(std::string shortName) {
        _shortName = std::move(shortName);
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
    OptionBuilder<T> &OptionBuilder<T>::required() {
        _required = true;
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &OptionBuilder<T>::conflictsWith(const std::string &name) {
        _conflictsWith.emplace_back(name);
        return *this;
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template DisableIfBool<OptionBuilder<T> &, U>
    OptionBuilder<T>::defaultValue(T defaultValue) {
        value = defaultValue;
        return *this;
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template DisableIfBool<OptionBuilder<T> &, U>
    OptionBuilder<T>::implicitValue(T implicitValue) {
        _implicitValue = implicitValue;
        return *this;
    }

    template<typename T>
    OptionBuilder<T> &
    OptionBuilder<T>::positionalValue(int amount, po::positional_options_description &positionalDesc) {
        _positionalAmount = amount;
        _positionalDesc = positionalDesc;
        return *this;
    }

    template<typename T>
    Option<T> OptionBuilder<T>::build() {
        return Option(*this);
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template EnableIfBool<OptionFlag, U> OptionBuilder<T>::buildFlag() {
        return OptionFlag(*this);
    }

    template<typename T>
    template<typename U>
    typename OptionBuilder<T>::template DisableIfBool<OptionValidator<T>, U>
    OptionBuilder<T>::build(Validator validator) {
        return OptionValidator(*this, validator);
    }
}

#endif //EVGET_INCLUDE_COMMANDLINEOPTIONBUILDER_H
