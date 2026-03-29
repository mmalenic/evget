#include "evget/interval_tracker.h"

std::optional<std::chrono::microseconds> evget::IntervalTracker::Interval(std::uint64_t time) {
    if (!previous_.has_value() || time < *previous_) {
        previous_ = time;
        return std::nullopt;
    }

    std::chrono::microseconds interval{time - *previous_};
    previous_ = time;

    return interval;
}
