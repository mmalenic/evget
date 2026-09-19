# Architecture

The architecture of evget is split across common functionality and platform-specific functionality. The format of the
data recorded is the same across all platforms, and is split into structures and fields. The structures format mouse
click, move and scroll events, as well as key press events.

The following table describes the data associated with a mouse move event:

| Field                     | Type                                                                | Description                                                                                                                         |
| ------------------------- | ------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| `interval`                | Microseconds                                                        | The interval between this event and the previous event in microseconds. This may be empty if this is the first event in the series. |
| `timestamp`               | Timestamp                                                           | The ISO8601 UTC timestamp of the event.                                                                                             |
| `position_x`              | Float                                                               | The x position of the mouse pointer at the time of the event. This may be absolute or relative depending on the platform.           |
| `position_y`              | Float                                                               | The y position of the mouse pointer at the time of the event. This may be absolute or relative depending on the platform.           |
| `device_name`             | String                                                              | The name of the device, which is defined by the platform.                                                                           |
| `focus_window_name`       | String                                                              | The name of the window in focus at the time of the event.                                                                           |
| `focus_window_position_x` | Float                                                               | The x position of the window in focus.                                                                                              |
| `focus_window_position_y` | Float                                                               | The y position of the window in focus.                                                                                              |
| `focus_window_width`      | Float                                                               | The width of the window in focus.                                                                                                   |
| `focus_window_height`     | Float                                                               | The height of the window in focus.                                                                                                  |
| `screen`                  | Integer                                                             | The display the event occurred on, numbered from 0. This will always be 0 if there is only one display.                             |
| `device_type`             | Enum of `Mouse`, `Keyboard`, `Touchpad`, `Touchscreen` or `Unknown` | The type of the device that generated the event.                                                                                    |

In addition to fields on move events, a mouse scroll event also has the following fields:

| Field               | Type  | Description                                                                                                                               |
| ------------------- | ----- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| `scroll_vertical`   | Float | The platform-defined amount that was vertically scrolled. A positive value indicates a scroll up and a negative value a scroll down.      |
| `scroll_horizontal` | Float | The platform-defined amount that was horizontally scrolled. A positive value indicates a scroll left and a negative value a scroll right. |

A mouse click event has all the fields of a mouse move event, and the following:

| Field           | Type                                   | Description                                                                                                                                                                                    |
| --------------- | -------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `button_id`     | Integer                                | The platform-defined id of the button that the event corresponds to.                                                                                                                           |
| `button_name`   | String                                 | The name of the button that the event corresponds to.                                                                                                                                          |
| `button_action` | Enum of `Press`, `Release` or `Repeat` | Whether the event was caused by a button press, release, or repeat. A repeat occurs when the OS re-triggers an event while a button is being held, such as when holding a key on the keyboard. |

A key event has the same fields as a mouse click event, including fields present on mouse moves, as well as the following field:

| Field       | Type            | Description                                                |
| ----------- | --------------- | ---------------------------------------------------------- |
| `character` | UTF-8 character | The UTF-8 character that is associated with the key event. |

## X11 behaviour

The data source when using X11 is the X11 [api][x11-api] functions. Namely, data is sourced using the XI2 extension
from `XIRawEvent` and functions like `XIQueryPointer`. The x and y position of the pointer is absolute, and button
names, ids, screens, and scroll amounts are sourced from the raw event.

## Windows touch behaviour

For windows, touch events are source from the Raw Input HID report. Like the rest of evget, pen inputs are not captured.
Touch events on windows produce the same rows as the libinput backend, i.e. mouse click press, mouse move, mouse click 
release.

Touch positions are based in screen pixel deltas on the `screen` where the touch event occurred. Windows also creates
pointer input from touch and touchpad events, so touch events on windows carry duplicate mouse move rows. This is
required because they capture different kinds of information. The touch event represents what was touched on the screen,
whereas the mouse event shows what happens to the pointer after this. It is not possible to precisely correlate these,
so evget doesn't attempt to and just records both.

The values can be used to determine pointer behaviour across devices, or to capture windows specific decision about the
pointer, such as two-finger scrolling, press and hold right click, acceleration, etc. The `device_type` or `device_id`
can be used to distinguish these events. If windows thinks the device touch is unintentional, it will mark the event
as such, and these events will not be recorded by evget.

[x11-api]: https://www.x.org/releases/X11R7.6/doc/
