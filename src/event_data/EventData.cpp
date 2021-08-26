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

#include <tuple>
#include <UnsupportedOperationException.h>
#include <event_data/EventData.h>
#include "EventData/EventData.h"

using namespace std;

EventData::EventData() : dataCreated{false}, eventData{} {
}

void EventData::addEntry(EventDeviceFormat format, DataType type, const string& entry) {
    if (dataCreated) {
        throw UnsupportedOperationException{"Attempted to modify after call to stop."};
    }
    eventData.emplace_back(format, type, entry);
}

EventData EventData::finish() {
    dataCreated = true;
    return *this;
}

EventData::iterator EventData::begin() noexcept {
    return eventData.begin();
}

EventData::iterator EventData::end() noexcept {
    return eventData.end();
}

size_t EventData::numberOfFields() const {
    return nFields;
}
