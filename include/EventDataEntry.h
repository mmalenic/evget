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

#ifndef EVGET_INCLUDE_EVENTDATAENTRY_H
#define EVGET_INCLUDE_EVENTDATAENTRY_H

#include <string>

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
 * Types to represent events.
 */
enum DataType {
    text,
    eventtime,
    integer
};

/**
 * Class represents an event data entry, i.e. one field.
 */
class EventDataEntry {
public:
    /**
     * Create the entry.
     * @param eventDeviceFormat format
     * @param dataType type
     * @param entry entry
     */
    EventDataEntry(EventDeviceFormat eventDeviceFormat, DataType dataType, std::string entry);
/**
     * Get the event device format.
     * @return event device format
     */
    [[nodiscard]] EventDeviceFormat getEventDeviceFormat() const;

    /**
     * Get the data type.
     * @return data type
     */
    [[nodiscard]] DataType getDataType() const;

    /**
     * Get the entry.
     * @return entry
     */
    [[nodiscard]] const std::string& getEntry() const;

private:
    EventDeviceFormat eventDeviceFormat;
    DataType dataType;
    std::string entry;
};

#endif //EVGET_INCLUDE_EVENTDATAENTRY_H
