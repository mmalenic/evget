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

#ifndef EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H
#define EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H

#include <filesystem>
#include <utility>

#include "XInputEvent.h"
#include "XInputHandler.h"
#include "XWrapper.h"
#include "evgetcore/EventLoop.h"

namespace EvgetX11 {
namespace asio = boost::asio;

/**
 * Class represents processing evgetx11 system events.
 */
template <boost::asio::execution::executor E, XWrapper XWrapper>
class EventLoopX11 : public EvgetCore::EventLoop<E, XInputEvent> {
public:
    /**
     * Create the system events.
     */
    explicit EventLoopX11(E& context, XInputHandler<XWrapper> xInputHandler);

    boost::asio::awaitable<void> eventLoop() override;
    void notify(XInputEvent event) override;
    void registerEventListener(EvgetCore::EventListener<XInputEvent>& eventListener) override;
    void stop() override;
    asio::awaitable<void> start() override;
    asio::awaitable<bool> isStopped();

private:
    std::optional<std::reference_wrapper<EvgetCore::EventListener<XInputEvent>>> _eventListener;
    XInputHandler<XWrapper> handler;
    std::reference_wrapper<E> context;

    std::atomic<bool> stopped{false};
};

template <boost::asio::execution::executor E, XWrapper XWrapper>
asio::awaitable<bool> EventLoopX11<E, XWrapper>::isStopped() {
    co_return stopped.load();
}

template <boost::asio::execution::executor E, XWrapper XWrapper>
asio::awaitable<void> EventLoopX11<E, XWrapper>::start() {
    asio::co_spawn(context, this->eventLoop(), boost::asio::detached);

    co_return;
}

template <boost::asio::execution::executor E, XWrapper XWrapper>
void EventLoopX11<E, XWrapper>::stop() {
    stopped.store(true);
}

template <boost::asio::execution::executor E, XWrapper XWrapper>
void EventLoopX11<E, XWrapper>::registerEventListener(EvgetCore::EventListener<XInputEvent>& eventListener) {
    _eventListener = eventListener;
}

template <boost::asio::execution::executor E, XWrapper XWrapper>
void EventLoopX11<E, XWrapper>::notify(XInputEvent event) {
    if (_eventListener.has_value()) {
        _eventListener->get().notify(std::move(event));
    }
}

template <boost::asio::execution::executor E, XWrapper XWrapper>
EventLoopX11<E, XWrapper>::EventLoopX11(E& context, XInputHandler<XWrapper> xInputHandler)
    : EvgetCore::EventLoop<E, XInputEvent>{context}, handler{xInputHandler}, context{context} {}

template <boost::asio::execution::executor E, XWrapper XWrapper>
boost::asio::awaitable<void> EventLoopX11<E, XWrapper>::eventLoop() {
    while (!co_await isStopped()) {
        this->notify(std::move(handler.getEvent()));
    }
    co_return;
}

}  // namespace EvgetX11

#endif  // EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H
