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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "SystemEventsLinux.h"

namespace asio = boost::asio;

using namespace std;


SystemEventsLinux::SystemEventsLinux(boost::fibers::buffered_channel<input_event>& sendChannel) : SystemEvents(sendChannel), fd{} {
}

boost::asio::awaitable<input_event> SystemEventsLinux::readSystemEvent() {
    if (!fd.has_value()) {
        throw UnsupportedOperationException("File descriptor has not been initialized.");
    }
    return boost::asio::awaitable<input_event>();
}

void SystemEventsLinux::setup() {
    SystemEvents::setup();
}

void SystemEventsLinux::shutdown() {
    SystemEvents::shutdown();
}
