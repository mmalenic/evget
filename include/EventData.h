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

#ifndef EVGET_INCLUDE_EVENTDATA_H
#define EVGET_INCLUDE_EVENTDATA_H

#include <any>
#include <vector>
#include <string>
#include <variant>

/**
 * Types to represent events.
 */
enum DataType {
    text,
    eventtime,
    integer
};

/**
 * Device formats for event data.
 */
enum EventDeviceFormat {
    raw,
    mousemove,
    mouseclick,
    mousewheel,
    key,
    touch
};

/**
 * Data container to represent event data.
 */
class EventData {
public:
    typedef typename std::vector<std::tuple<EventDeviceFormat, DataType, const std::string&>>::iterator iterator;
    /**
     * Create the event data.
     */
    EventData();

    /**
     * Add an entry to the data container
     * @param type type
     * @param entry entry
     */
    void addEntry(EventDeviceFormat format, DataType type, const std::string& entry);

    /**
     * Get the resulting event data.
     * @return event data
     */
    EventData finish();

    /**
     * Iterator begin.
     * @return iterator begin
     */
    [[nodiscard]] iterator begin() noexcept;

    /**
     * Iterator end.
     * @return iterator end
     */
    [[nodiscard]] iterator end() noexcept;

private:
    bool dataCreated;
    std::vector<std::tuple<EventDeviceFormat, DataType, const std::string&>> eventData;
};

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

#endif //EVGET_INCLUDE_EVENTDATA_H
