// Copyright (C) Marko Malenic 2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef EVGET_INCLUDE_UNSUPPORTEDOPERATIONEXCEPTION_H
#define EVGET_INCLUDE_UNSUPPORTEDOPERATIONEXCEPTION_H

#include <string>

/**
 * Exception used for an unsupported operation.
 */
class UnsupportedOperationException : public std::exception {
public:
    /**
     * Create exception with message.
     * @param message message
     */
    explicit UnsupportedOperationException(std::string message = "Unsupported Operation.");
    [[nodiscard]] const char* what() const noexcept override;

private:
    std::string message;
};

#endif //EVGET_INCLUDE_UNSUPPORTEDOPERATIONEXCEPTION_H
