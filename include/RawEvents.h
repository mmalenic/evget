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

#ifndef EVGET_INCLUDE_RAWEVENTS_H
#define EVGET_INCLUDE_RAWEVENTS_H

#include <vector>
#include <chrono>
#include "EventHandler.h"

/**
 * Class represents processing the raw system events.
 * @tparam T type of events to process
 */
template <typename T>
class RawEvents {
public:
    /**
     * Create the raw event processor.
     * @param bufferSize used with buffer to store events into
     * @param drainFrequency how often to drain events
     */
    RawEvents(unsigned char bufferSize, std::chrono::seconds drainFrequency, EventHandler<T> &eventHandler);

    /**
     * Set up and run the event loop.
     */
    virtual void eventLoop() = 0;

    /**
     * Drains raw event to listeners.
     */
    virtual void drainRawEvents() = 0;

    virtual ~RawEvents() = default;
    RawEvents(const RawEvents&) = default;
    RawEvents(RawEvents&&) = default;
    virtual RawEvents& operator=(const RawEvents&) = default;
    virtual RawEvents& operator=(RawEvents&&) = default;

private:
    unsigned char bufferSize;
    std::chrono::seconds drainFrequency;
    EventHandler<T> &eventHandler;
};

#endif //EVGET_INCLUDE_RAWEVENTS_H
