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
//

#ifndef EVGET_STORAGE_H
#define EVGET_STORAGE_H

#include "evgetcore/Error.h"
#include "evgetcore/Event/Data.h"

namespace EvgetCore::Storage {

/**
 * An interface which represents storing data.
 */
class Store {
public:
    /**
     * Store the event data.
     */
    virtual Result<void> store(Event::Data event) = 0;

    Store() = default;

    virtual ~Store() = default;

    Store(const Store&) = delete;
    Store(Store&&) noexcept = delete;
    Store& operator=(const Store&) = delete;
    Store& operator=(Store&&) noexcept = delete;
};
}  // namespace EvgetCore::Storage

#endif  // EVGET_STORAGE_H
