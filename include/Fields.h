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

#ifndef EVGET_INCLUDE_FIELDS_H
#define EVGET_INCLUDE_FIELDS_H

#include <string>
#include <vector>

class EventType {
public:
    enum class FormatType {
        SYSTEM,
        STANDARD
    };

    void include(std::string field, FormatType type);
    std::vector<std::string> getFields();
    FormatType getType(std::string field);
};

class MouseMove : public EventType {};
class MouseClick : public EventType {};
class MouseWheel : public EventType {};
class Key : public EventType {};
class Touch : public EventType {};

/**
 * Describes the fields that can be stored.
 */
class Fields {
public:

    /**
     * Construct fields from string.
     * @param fields fields string
     */
    explicit Fields(std::string fields);
};

#endif //EVGET_INCLUDE_FIELDS_H
