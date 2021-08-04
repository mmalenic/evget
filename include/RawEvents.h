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
#include <boost/circular_buffer.hpp>
#include "EventHandler.h"
#include "ShutdownHandler.h"

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
    RawEvents(size_t bufferSize, size_t minimumDrainSize, std::chrono::seconds drainFrequency, EventHandler<T> &eventHandler, ShutdownHandler& shutdownHandler);

    /**
     * Get buffer size.
     * @return buffer size
     */
    [[nodiscard]] size_t getBufferSize() const;

    /**
     * Get minimum drain size.
     * @return minimum drain size
     */
    [[nodiscard]] size_t getMinimumDrainSize() const;

    /**
     * Get drain frequency.
     * @return drain frequency
     */
    [[nodiscard]] const std::chrono::seconds& getDrainFrequency() const;

    /**
     * Get event handler.
     * @return event handler
     */
    EventHandler<T>& getEventHandler() const;

    /**
     * Set up and run the event loop.
     */
    virtual void eventLoop();

    /**
     * Drains raw event to listeners.
     */
    virtual void drainRawEvents();

    /**
     * Set up the event loop.
     */
    virtual void setup();

    /**
     * Cleanup after event loop.
     */
    virtual void shutdown();

    virtual T readRawEvent() = 0;

    virtual ~RawEvents() = default;
    RawEvents(const RawEvents&) = default;
    RawEvents(RawEvents&&) noexcept = default;
    RawEvents& operator=(const RawEvents&) = default;
    RawEvents& operator=(RawEvents&&) noexcept = default;

private:
    const size_t bufferSize;
    const size_t minimumDrainSize;
    const std::chrono::seconds drainFrequency;
    EventHandler<T>& eventHandler;
    ShutdownHandler& shutdownHandler;

    boost::circular_buffer<T> buffer;
};

template<typename T>
RawEvents<T>::RawEvents(size_t bufferSize, size_t minimumDrainSize, std::chrono::seconds drainFrequency, EventHandler<T> &eventHandler, ShutdownHandler& shutdownHandler) :
    bufferSize{bufferSize}, minimumDrainSize{minimumDrainSize}, drainFrequency{drainFrequency}, eventHandler{eventHandler}, shutdownHandler{shutdownHandler}, buffer{bufferSize} {
}

template<typename T>
size_t RawEvents<T>::getBufferSize() const {
    return bufferSize;
}

template<typename T>
const std::chrono::seconds& RawEvents<T>::getDrainFrequency() const {
    return drainFrequency;
}

template<typename T>
EventHandler<T>& RawEvents<T>::getEventHandler() const {
    return eventHandler;
}

template<typename T>
void RawEvents<T>::eventLoop() {
    setup();
    auto start_time = std::chrono::high_resolution_clock::now();
    while (!shutdownHandler.shouldShutdown()) {
        buffer.push_back(readRawEvent());
        if (shutdownHandler.shouldShutdown() || (buffer.size() >= minimumDrainSize && std::chrono::high_resolution_clock::now() - start_time >= drainFrequency)) {
            drainRawEvents();
            start_time = std::chrono::high_resolution_clock::now();
        }
    }
    shutdown();
}

template<typename T>
void RawEvents<T>::drainRawEvents() {
    std::vector<T> events = std::vector(buffer.begin(), buffer.end());
    buffer.clear();
    eventHandler.processEvents(events);
}

template<typename T>
size_t RawEvents<T>::getMinimumDrainSize() const {
    return minimumDrainSize;
}

template<typename T>
void RawEvents<T>::setup() {
}

template<typename T>
void RawEvents<T>::shutdown() {
}

#endif //EVGET_INCLUDE_RAWEVENTS_H
