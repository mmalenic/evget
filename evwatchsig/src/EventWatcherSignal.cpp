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

#include "evwatchsig/EventWatcherSignal.h"

#include <csignal>

#include "evwatch/EventWatcherException.h"

EvWatch::Sig::EventWatcherSignal::EventWatcherSignal(std::initializer_list<int> registerSignals, int flags) {
    struct sigaction sigIntHandler {};

    sigIntHandler.sa_handler = signalHandler;
    if (sigemptyset(&sigIntHandler.sa_mask) == -1) {
        throw EventWatcherException(errno, "Setting sig handler mask failed.");
    }
    sigIntHandler.sa_flags = flags;

    for (auto signal : registerSignals) {
        if (sigaction(signal, &sigIntHandler, nullptr) == -1) {
            throw EventWatcherException(errno, "Setting sig action failed.");
        }
    }
}

void EvWatch::Sig::EventWatcherSignal::signalHandler([[maybe_unused]] int _) {
    EventWatcher::setEvent();
}
