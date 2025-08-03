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

#include "evget/EventLoop.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XInputHandler.h"
#include "evgetx11/XWrapper.h"

namespace EvgetX11 {
namespace asio = boost::asio;

/**
 * Class represents processing evgetx11 system events.
 */
class EventLoopX11 : public evget::EventLoop<XInputEvent> {
public:
    /**
     * Create the system events.
     */
    explicit EventLoopX11(XInputHandler xInputHandler);

    evget::Result<void> notify(XInputEvent event) override;
    void registerEventListener(evget::EventListener<XInputEvent>& eventListener) override;
    void stop() override;
    asio::awaitable<evget::Result<void>> start() override;
    asio::awaitable<bool> isStopped();

private:
    std::optional<std::reference_wrapper<evget::EventListener<XInputEvent>>> _eventListener;
    XInputHandler handler;

    std::atomic<bool> stopped{false};
};
}  // namespace EvgetX11

#endif  // EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H
