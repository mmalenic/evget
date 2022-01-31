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

#ifndef EVGET_INCLUDE_SYSTEMEVENTLOOP_H
#define EVGET_INCLUDE_SYSTEMEVENTLOOP_H

#include <vector>
#include <chrono>
#include <boost/fiber/buffered_channel.hpp>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include "ShutdownHandler.h"
#include "Task.h"
#include "SystemEvent.h"
#include "EventListener.h"

namespace evget {
    
    namespace asio = boost::asio;
    
    /**
     * Class represents processing the system events.
     * @tparam T type of events to process
     */
    template<asio::execution::executor E, typename T>
    class SystemEventLoop : public Task<E>, public EventListener<T> {
    public:
        /**
         * Create the system events class.
         */
        explicit SystemEventLoop(E& context);

        /**
         * Set up and run the event loop.
         */
        virtual asio::awaitable<void> eventLoop() = 0;

        /**
         * Register listeners to notify.
         * @param systemEventListener lister
         */
        void registerSystemEventListener(EventListener<T>& systemEventListener);

        asio::awaitable<void> start() override;
        void notify(T event) override;

        virtual ~SystemEventLoop() = default;

    protected:
        SystemEventLoop(SystemEventLoop&&) noexcept = default;
        SystemEventLoop& operator=(SystemEventLoop&&) noexcept = default;

        SystemEventLoop(const SystemEventLoop&) = default;
        SystemEventLoop& operator=(const SystemEventLoop&) = default;

    private:
        std::vector<std::reference_wrapper<EventListener<T>>> eventListeners;
    };

    template<asio::execution::executor E, typename T>
    asio::awaitable<void> SystemEventLoop<E, T>::start() {
        co_await Task<E>::start();
        co_await eventLoop();
        this->stop();
        co_return;
    }

    template<asio::execution::executor E, typename T>
    void SystemEventLoop<E, T>::notify(T event) {
        for (const auto& listener : eventListeners) {
            listener.get().notify(std::move(event));
        }
    }

    template<asio::execution::executor E, typename T>
    void SystemEventLoop<E, T>::registerSystemEventListener(EventListener<T>& systemEventListener) {
        eventListeners.push_back(systemEventListener);
    }

    template<asio::execution::executor E, typename T>
    SystemEventLoop<E, T>::SystemEventLoop(E& context) : Task<E>{context},
        eventListeners{} {
    }
}

#endif //EVGET_INCLUDE_SYSTEMEVENTLOOP_H
