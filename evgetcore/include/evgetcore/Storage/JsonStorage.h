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

#ifndef EVGET_JSONSTORAGE_H
#define EVGET_JSONSTORAGE_H

#include <boost/asio.hpp>

#include <fstream>

#include "Store.h"
#include "evgetcore/Error.h"

namespace EvgetCore::Storage {

/**
 * A storage class which prints events to stdout.
 */
class JsonStorage : public Store {
public:
    explicit JsonStorage(std::unique_ptr<std::ostream> ostream);
    explicit JsonStorage(std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> ostream);

    Result<void> store(Event::Data event) override;

private:
    std::variant<std::unique_ptr<std::ostream>, std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>>
        ostream;
};
}  // namespace EvgetCore::Storage

#endif  // EVGET_JSONSTORAGE_H
