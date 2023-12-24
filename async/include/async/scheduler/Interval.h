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

#include <iostream>

#include "async/Util.h"

namespace Async {

namespace asio = boost::asio;

/**
 * \brief A repeating timer indefinitely repeats until it is stopped. This class does not have thread-safe
 * functionality.
 */
class Interval {
public:
    /**
     * \brief construct a repeating timer.
     * \param period period for the interval
     */
    explicit Interval(std::chrono::seconds period);

    /**
     * \brief Completes when the next period in the interval has been reached. If a tick has been
     * missed, then the timer keeps firing until the time has caught up. Not thread-safe.
     */
    asio::awaitable<Result<void, boost::system::error_code>> tick();

    /**
     * \brief Reset the interval to expire one period after the current time. Not thread-safe.
     */
    void reset();

    /**
     * \brief Get the timer's period.
     * \return period.
     */
    [[nodiscard]] std::chrono::seconds period() const;

private:
    std::chrono::seconds _period;
    std::optional<asio::steady_timer> timer{};
};
}

#endif //TIMER_H
