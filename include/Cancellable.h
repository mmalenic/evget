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

#ifndef EVGET_INCLUDE_CANCELLABLE_H
#define EVGET_INCLUDE_CANCELLABLE_H

#include <atomic>

/**
 * Cancellable class.
 */
class Cancellable {
public:
    /**
     * Create cancellable object.
     */
    Cancellable();

    /**
     * Cancel the class.
     */
    void cancel();

    /**
     * Check if cancelled.
     * @return is cancelled.
     */
    [[nodiscard]] bool isCancelled() const;

private:
    std::atomic<bool> cancelled;
};

#endif //EVGET_INCLUDE_CANCELLABLE_H
