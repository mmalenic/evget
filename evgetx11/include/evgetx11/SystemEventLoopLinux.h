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
#include "evgetcore/SystemEventLoop.h"
#include "evgetcore/SystemEvent.h"
#include "evgetx11/XInputEvent.h"
#include "evgetx11/XInputHandler.h"
#include <utility>

namespace EvgetX11 {
    namespace asio = boost::asio;

    /**
     * Class represents processing evgetx11 system events.
     */
    template<boost::asio::execution::executor E>
    class SystemEventLoopLinux : public EvgetCore::SystemEventLoop<E, XInputEvent> {
    public:
        /**
         * Create the system events.
         */
        explicit SystemEventLoopLinux(E& context, XInputHandler xInputHandler);

        boost::asio::awaitable<void> eventLoop() override;

    private:
        XInputHandler handler;
    };

    template<boost::asio::execution::executor E>
    SystemEventLoopLinux<E>::SystemEventLoopLinux(
        E& context,
        XInputHandler xInputHandler
    ) : EvgetCore::SystemEventLoop<E, XInputEvent>{context},
        handler{xInputHandler} {
    }

    template<boost::asio::execution::executor E>
    boost::asio::awaitable<void> SystemEventLoopLinux<E>::eventLoop() {
        while (!this->isCancelled()) {
            this->notify(std::move(handler.getEvent()));
        }
        co_return;
    }
}

#endif // EVGET_INCLUDE_LINUX_SYSTEMEVENTLOOPLINUX_H
