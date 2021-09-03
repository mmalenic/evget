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

#include <string_view>

/**
 * Represents a command line option.
 */
class CommandLineOption {
public:
    /**
     * Create command line option.
     * @param shortName short name
     * @param longName long name
     * @param description description
     */
    constexpr CommandLineOption(
        std::string_view shortName,
        std::string_view longName,
        std::string_view description
    ) : shortName{shortName}, longName{longName}, description{description} {
    }

    /**
     * Get short name.
     * @return short name
     */
    [[nodiscard]] constexpr std::string_view getShortName() const;

    /**
     * Get long name.
     * @return long name
     */
    [[nodiscard]] constexpr std::string_view getLongName() const;

    /**
     * Get description.
     * @return description
     */
    [[nodiscard]] constexpr std::string_view getDescription() const;

private:
    const std::string_view shortName;
    const std::string_view longName;
    const std::string_view description;
};

#endif //EVGET_INCLUDE_COMMANDLINEOPTION_H
