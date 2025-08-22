#ifndef EVGETX11_XINPUTEVENT_H
#define EVGETX11_XINPUTEVENT_H

#include <X11/Xlib.h>

#include "evget/event/schema.h"
#include "evgetx11/XWrapper.h"

namespace evgetx11 {
class XInputEvent {
public:
    /**
     * Get the date time of the event.
     */
    [[nodiscard]] const evget::TimestampType& GetTimestamp() const;

    /**
     * Check if viewData and getEventType is safe to call.
     */
    [[nodiscard]] bool HasData() const;

    /**
     * Must check if data is available first with hasData.
     */
    [[nodiscard]] int GetEventType() const;

    /**
     * A non owning reference to the data in the event cookie. Must check if data is available first
     * with hasData.
     */
    template <typename T>
    [[nodiscard]] const T& ViewData() const;

    /**
     * Create a XInputEvent by getting the next event from the display. Events received depend on
     * the event mask set on the display. This function will block if there are no events on the event
     * queue.
     */
    static XInputEvent NextEvent(XWrapper& x_wrapper);

private:
    explicit XInputEvent(XWrapper& x_wrapper);

    XEvent event_;
    evget::TimestampType timestamp_;
    XEventPointer cookie_;
};

template <typename T>
const T& evgetx11::XInputEvent::ViewData() const {
    return *static_cast<T*>(cookie_->data);
}
}  // namespace evgetx11

#endif
