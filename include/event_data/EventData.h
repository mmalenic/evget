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
#include "EventDataEntry.h"

/**
 * Data container to represent event data.
 */
class EventData {
public:
    typedef typename std::vector<EventDataEntry>::iterator iterator;

    /**
     * Construct the event data
     * @param nFields number of fields.
     */
    explicit EventData(size_t nFields);

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

    /**
     * Include a field.
     * @param position at position
     * @param entry with entry
     */
    void includeField(size_t position, std::string entry);

    /**
     * Get the entry.
     * @param position at position
     * @return entry
     */
    EventDataEntry getEntry(size_t position);

    /**
     * Get the name of the data.
     * @return name
     */
    virtual std::string name() = 0;

    /**
     * Get the fields.
     * @return fields
     */
    virtual std::vector<std::string> fields() = 0;

private:
    std::vector<EventDataEntry> eventData;
};

class MouseMove : public EventData {
public:
    static constexpr size_t NUM_FIELDS = 1;
    enum class Field {

    };

    MouseMove();

    void includeField(Field field, std::string entry);
};
class MouseClick : public EventData {};
class MouseWheel : public EventData {};
class Key : public EventData {};
class Touch : public EventData {};

#endif //EVGET_INCLUDE_EVENTDATA_H
