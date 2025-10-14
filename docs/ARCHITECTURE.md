# Architecture 

The architecture of evget is split across common functionality and platform-specific functionality. The format of the
data recorded is the same across all platforms, and is split into structures and fields. The structures format mouse
click, move and scroll events, as well as key press events.

The following table describes the data associated with a mouse move event:

| Field                     | Type                                                                | Description                                                                                                                         |
|---------------------------|---------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------|
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
