/**
 * \file interval_tracker.h
 * \brief Tracks time intervals between consecutive events.
 */

#ifndef EVGET_INTERVAL_TRACKER_H
#define EVGET_INTERVAL_TRACKER_H

#include <chrono>
#include <cstdint>
#include <optional>

namespace evget {

/**
 * \brief Tracks the time interval between consecutive events in microseconds.
 */
class IntervalTracker {
public:
    /**
     * \brief Calculate the time interval since the last event using the event's own timestamp in microseconds.
     *
     * If the input time is less than the time of the previous event `std::nullopt` is returned.
     *
     * \param time current event time
     * \return optional interval in microseconds, `std::nullopt` if no previous time
     */
    std::optional<std::chrono::microseconds> Interval(std::uint64_t time);

private:
    std::optional<std::uint64_t> previous_;
};

} // namespace evget

#endif // EVGET_INTERVAL_TRACKER_H
