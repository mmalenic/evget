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

#include "evgetx11/XInputEvent.h"

EvgetX11::XInputEvent::XInputEvent(XWrapper& xWrapper)
    : event{xWrapper.nextEvent()}, timestamp{std::chrono::system_clock::now()}, cookie{xWrapper.eventData(event)} {}

bool EvgetX11::XInputEvent::hasData() const {
    return cookie != nullptr;
}

int EvgetX11::XInputEvent::getEventType() const {
    return cookie->evtype;
}

const EvgetCore::Event::SchemaField::Timestamp& EvgetX11::XInputEvent::getTimestamp() const {
    return timestamp;
}

EvgetX11::XInputEvent EvgetX11::XInputEvent::nextEvent(XWrapper& xWrapper) {
    return XInputEvent{xWrapper};
}