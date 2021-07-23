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

#ifndef EVGET_INCLUDE_EVENTHANDLER_H
#define EVGET_INCLUDE_EVENTHANDLER_H

#include <vector>
#include "DataTransformer.h"
#include "Storage.h"
/**
 * Class represents a listener for events.
 * @tparam T type of data
 */
template <typename T>
class EventHandler {
public:
    /**
     * Create the event listener with storage.
     * @param storage storage
     */
    EventHandler(Storage storage, DataTransformer<T> transformer);

    /**
     * Process the data and give it to the storage.
     * @param data data to process
     */
    virtual void processData(std::vector<T> data) = 0;

    virtual ~EventHandler() = default;
    EventHandler(const EventHandler&) = default;
    EventHandler(EventHandler&&) = default;
    virtual EventHandler& operator=(const EventHandler&) = default;
    virtual EventHandler& operator=(EventHandler&&) = default;

private:
    Storage storage;
    DataTransformer<T> transformer;
};

#endif //EVGET_INCLUDE_EVENTHANDLER_H
