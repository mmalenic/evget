/**
 * \file input_event.h
 * \brief Input event wrapper with timestamp measurement and data access.
 */

#ifndef EVGET_INPUT_EVENT_H
#define EVGET_INPUT_EVENT_H

#include "event/schema.h"

namespace evget {

/**
 * \brief Represents an input event with a timestamp and event data.
 * \tparam T the inner event data type
 */
template <typename T>
class InputEvent {
public:
    /**
     * \brief Create an input event.
     * @param event inner event
     */
    explicit InputEvent(T event);

    /**
     * \brief Get the timestamp of when the event occurred.
     * \return reference to the event timestamp
     */
    [[nodiscard]] const TimestampType& GetTimestamp() const;

    /**
     * \brief Get a reference to the event data.
     * \return reference to the event data
     */
    [[nodiscard]] T& ViewData();

    /**
     * \brief Get the inner data of this class by moving out of the class.
     * \return the timestamp and inner event data
     */
    [[nodiscard]] std::pair<TimestampType, T> IntoInner() &&;

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
    T event_;
    TimestampType timestamp_{Now()};
    std::optional<std::uint64_t> previous_;
};

} // namespace evget

template <typename T>
evget::InputEvent<T>::InputEvent(T event) : event_{std::move(event)} {}

template <typename T>
const evget::TimestampType& evget::InputEvent<T>::GetTimestamp() const {
    return timestamp_;
}

template <typename T>
T& evget::InputEvent<T>::ViewData() {
    return event_;
}

template <typename T>
std::pair<evget::TimestampType, T> evget::InputEvent<T>::IntoInner() && {
    return {std::move(timestamp_), std::move(event_)};
}

template <typename T>
std::optional<std::chrono::microseconds> evget::InputEvent<T>::Interval(std::uint64_t time) {
    if (!previous_.has_value() || time < *previous_) {
        previous_ = time;
        return std::nullopt;
    }

    std::chrono::microseconds interval{time - *previous_};
    previous_ = time;

    return interval;
}

#endif // EVGET_INPUT_EVENT_H
