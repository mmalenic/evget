/**
 * \file input_event.h
 * \brief X11 input event wrapper with timestamp and data access.
 */

#ifndef EVGETX11_INPUT_EVENT_H
#define EVGETX11_INPUT_EVENT_H

#include <X11/Xlib.h>

#include "evget/event/schema.h"
#include "evget/input_event.h"
#include "evgetx11/x11_api.h"

namespace evgetx11 {
/**
 * \brief Represents an X11 input event with timestamp and event data.
 */
class InputEvent {
public:
    /**
     * \brief Get the timestamp of when the event occurred.
     * \return reference to the event timestamp
     */
    [[nodiscard]] const evget::TimestampType& GetTimestamp() const;

    /**
     * \brief Check if `ViewData` and `GetEventType` are safe to call.
     * \return true if event data is available, false otherwise
     */
    [[nodiscard]] bool HasData() const;

    /**
     * \brief Get the X11 event type. Must check if data is available first with HasData.
     * \return the X11 event type identifier
     */
    [[nodiscard]] int GetEventType() const;

    /**
     * \brief Get a non-owning reference to the event data. Must check if data is available first with HasData.
     * \tparam T The expected type of the event data
     * \return reference to the event data cast to the specified type
     */
    template <typename T>
    [[nodiscard]] const T& ViewData() const;

    /**
     * \brief Create an InputEvent by getting the next event from the display.
     *
     * Events received depend on the event mask set on the display.
     * This function will block if there are no events in the event queue.
     *
     * \param x_wrapper reference to the X11 API wrapper
     * \return the next input event from the display
     */
    static InputEvent NextEvent(X11Api& x_wrapper);

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
    /**
     * \brief Private constructor for creating InputEvent instances.
     * \param x_wrapper reference to the X11 API wrapper
     */
    explicit InputEvent(X11Api& x_wrapper);

    evget::InputEvent<XEvent> event_;
    XEventPointer cookie_;
};

template <typename T>
const T& InputEvent::ViewData() const {
    return *static_cast<T*>(cookie_->data);
}
} // namespace evgetx11

#endif
