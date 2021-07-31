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

#include "../include/RawEvents.h"
#include <vector>

using namespace std;

template<typename T>
RawEvents<T>::RawEvents(size_t bufferSize, size_t minimumDrainSize, std::chrono::seconds drainFrequency, EventHandler<T> &eventHandler, ShutdownHandler& shutdownHandler) :
    bufferSize{bufferSize}, minimumDrainSize{minimumDrainSize}, drainFrequency{drainFrequency}, eventHandler{eventHandler}, buffer{bufferSize}, shutdownHandler{shutdownHandler} {
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
    auto start_time = chrono::high_resolution_clock::now();
    while (!shutdownHandler.shouldShutdown()) {
        buffer.push_back(readRawEvent());
        if (shutdownHandler.shouldShutdown() || (buffer.size() >= minimumDrainSize && chrono::high_resolution_clock::now() - start_time >= drainFrequency)) {
            drainRawEvents();
            start_time = chrono::high_resolution_clock::now();
        }
    }
    shutdown();
}

template<typename T>
void RawEvents<T>::drainRawEvents() {
    vector<T> events = vector{buffer.begin(), buffer.end()};
    buffer.clear();
    eventHandler.processEvents(events);
}

template<typename T>
void RawEvents<T>::setup() {
}

template<typename T>
void RawEvents<T>::shutdown() {
}

template<typename T>
size_t RawEvents<T>::getMinimumDrainSize() const {
    return minimumDrainSize;
}
