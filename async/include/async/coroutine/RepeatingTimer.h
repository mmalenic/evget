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
//

#ifndef TIMER_H
#define TIMER_H

#include <boost/asio.hpp>
#include "async/Util.h"

namespace Async {

namespace asio = boost::asio;

/**
 * \brief A repeating timer indefinitely repeats until it is stopped.
 */
class RepeatingTimer {
public:
    using Err = Err<boost::system::error_code>;
    using Result = Result<void, boost::system::error_code>;

    /**
     * \brief construct a repeating timer.
     * \param interval interval for repeat
     */
    explicit RepeatingTimer(std::chrono::seconds interval);

    /**
     * \brief Await on the timer.
     * \param callback callback for each interval.
     * \return resulting indicating success or failure.
     */
    asio::awaitable<Result> await(Invocable<asio::awaitable<void>> auto&& callback);

    /**
     * \brief Stop the timer.
     */
    void stop();

private:
    static asio::awaitable<Result> repeat(asio::steady_timer* timer, std::chrono::seconds& interval, Invocable<asio::awaitable<void>> auto&& callback);

    std::chrono::seconds interval;
    std::optional<asio::steady_timer> timer;
};
}

#endif //TIMER_H
