#ifndef EVGET_ASYNC_SCHEDULER_INTERVAL_H
#define EVGET_ASYNC_SCHEDULER_INTERVAL_H

#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>
#include <optional>

#include "evget/error.h"

namespace evget {

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
    asio::awaitable<Result<void>> Tick();

    /**
     * \brief Reset the interval to expire one period after the current time. Not thread-safe.
     */
    void Reset();

    /**
     * \brief Get the timer's period.
     * \return period.
     */
    [[nodiscard]] std::chrono::seconds Period() const;

private:
    std::chrono::seconds period_{};
    std::optional<asio::steady_timer> timer_;
};
}  // namespace evget

#endif
