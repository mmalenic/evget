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

#ifndef EVGET_INCLUDE_STORAGE_H
#define EVGET_INCLUDE_STORAGE_H

#include "Event/Data.h"
#include "EventListener.h"
#include "Task.h"

namespace EvgetCore {
    namespace asio = boost::asio;

    /**
     * Storage class represents storing event data.
     */
    template<asio::execution::executor E>
    class Storage : Task<E>, EventListener<Event::Data> {
    public:
        explicit Storage(E& context);

        asio::awaitable<void> start() override;
        void notify(Event::Data event) override;
    };

    template<asio::execution::executor E>
    Storage<E>::Storage(E& context) : Task<E>{context} {
    }

    template<asio::execution::executor E>
    boost::asio::awaitable<void> Storage<E>::start() {
        Task<E>::start();
        co_return;
    }

    template<asio::execution::executor E>
    void Storage<E>::notify(Event::Data event) {
    }
}

#endif //EVGET_INCLUDE_STORAGE_H
