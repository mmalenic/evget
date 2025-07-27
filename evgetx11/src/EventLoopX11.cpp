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

#include "evgetx11/EventLoopX11.h"

#include <boost/asio/awaitable.hpp>

#include <utility>

#include "evgetcore/Error.h"
#include "evgetcore/EventListener.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XInputHandler.h"

EvgetX11::asio::awaitable<bool> EvgetX11::EventLoopX11::isStopped() {
    co_return stopped.load();
}

EvgetX11::asio::awaitable<EvgetCore::Result<void>> EvgetX11::EventLoopX11::start() {
    return this->eventLoop();
}

void EvgetX11::EventLoopX11::stop() {
    stopped.store(true);
}

void EvgetX11::EventLoopX11::registerEventListener(EvgetCore::EventListener<XInputEvent>& eventListener) {
    _eventListener = eventListener;
}

EvgetCore::Result<void> EvgetX11::EventLoopX11::notify(XInputEvent event) {
    if (_eventListener.has_value()) {
        return _eventListener->get().notify(std::move(event));
    }
    return {};
}

EvgetX11::EventLoopX11::EventLoopX11(XInputHandler xInputHandler) : handler{xInputHandler} {}

boost::asio::awaitable<EvgetCore::Result<void>> EvgetX11::EventLoopX11::eventLoop() {
    while (!co_await isStopped()) {
        auto result = this->notify(handler.getEvent());
        if (!result.has_value()) {
            co_return EvgetCore::Err{result.error()};
        }
    }
    co_return EvgetCore::Result<void>{};
}
