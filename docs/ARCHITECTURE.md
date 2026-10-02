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
| `screen`                  | String                                                              | The platform's name for the monitor the pointer is on, or null if it is on no monitor or the monitor is unknown.                    |
| `device_type`             | Enum of `Mouse`, `Keyboard`, `Touchpad`, `Touchscreen` or `Unknown` | The type of the device that generated the event.                                                                                    |

The `screen` is named the way the platform names monitors, so it tells monitors apart within a recording rather than
across machines. X11 stores the XRandR monitor name, such as `DP-1`, and Windows stores the display device name, such
as `\\.\DISPLAY1`. The libinput backend has no view of monitors, so it always stores null.

In addition to fields on move events, a mouse scroll event also has the following fields:

| Field               | Type  | Description                                                                                                                               |
| ------------------- | ----- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| `scroll_vertical`   | Float | The platform-defined amount that was vertically scrolled. A positive value indicates a scroll down and a negative value a scroll up.      |
| `scroll_horizontal` | Float | The platform-defined amount that was horizontally scrolled. A positive value indicates a scroll right and a negative value a scroll left. |

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

## Absent values

A value that a backend cannot determine is stored as SQL `NULL` and written as JSON `null`. For example,
`screen` is null when the pointer is on no monitor, and always null on libinput. The `--exclude-window-title`
option stores `focus_window_name` as null for every event while keeping the focus window position, size and `screen`.

## Synthetic input

Input that the OS synthesizes instead is stored with a `device_name` of `<backend>-injected`, and a
`device_type` from the kind of input. X11 stores `x11-injected` for input that the X server sends through its
XTEST devices.

Windows stores `windows-injected` for any Raw Input that arrives without a device handle. This includes input sent with
`SendInput`. Windows also send [pointer][raw-input-header] input it generates from a precision touchpad or a touchscreen
without a device handle. This means that `windows-injected` rows might be real events from a touchpad or touchscreen,
and evget cannot tell these apart. The touchpad and touchscreen rows that come directly from the device do have
the device's real name.

## X11 behaviour

The data source when using X11 is the X11 [api][x11-api] functions. Namely, data is sourced using the XI2 extension
from `XIRawEvent` and functions like `XIQueryPointer`. The x and y position of the pointer is absolute, and button
names, ids and scroll amounts are sourced from the raw event. The `screen` is the XRandR monitor that contains the
pointer.

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

## Windows runtime behaviour

### Elevation

evget runs with the rights it was started with and never asks for more. At startup it logs whether it is elevated, and
when it is not, the log line says that input sent to elevated windows is not captured.

Windows withholds input to a window with a higher integrity level from an unelevated process. While an elevated window
has focus, such as an administrator terminal, Task Manager or an installer, an unelevated evget stores no rows at all:
no keys, pointer moves, clicks or scrolls, and no rows from a touchpad's or touchscreen's own reports. Rows stored at
the moment focus changes may have a null focus window. Capture resumes once a window that is not elevated has focus. To
capture input sent to elevated windows, start evget with "Run as administrator" or from an administrator terminal.

The secure desktop, which shows the UAC prompt, the lock screen and the logon screen, is never captured, whether evget
is elevated or not. Elevating with the credentials of a different administrator account runs evget as that account, so a
per-user path in an explicit `-o`, such as one under `%LOCALAPPDATA%`, points into that account's profile.

### Running hidden

evget has no option to run in the background, so the shell or a scheduler starts it hidden, the same as on Linux. `-o`
defaults to `-`, which writes JSON to standard output, on every OS, so a hidden run needs an explicit `-o`. The window
evget captures input with is never shown, and does not appear in the taskbar or in Alt+Tab.

In PowerShell, `Start-Process` with `-WindowStyle Hidden` starts evget without a window and returns the prompt at once:

```powershell
Start-Process evget -ArgumentList '-o', 'events.sqlite' -WindowStyle Hidden
```

This has been checked with the classic console host as the default terminal, but not with Windows Terminal set as the
default terminal.

Task Scheduler can start evget each time the user logs on: create a task with an "At log on" trigger and "Run only when
user is logged on" selected, whose action runs evget with an explicit `-o`. evget then runs in a visible console window,
and closing that window stops it. Do not select "Run whether user is logged on or not", because that runs evget in
session 0, which receives no user input. Running evget as a Windows service is not supported for the same reason.

On Linux, closing the terminal that evget runs in sends it SIGHUP, which stops it. `nohup evget -o events.sqlite &`
keeps evget running after the terminal closes. A systemd user unit is another option, but it has not been tested.

### Stopping

A stop request that evget can catch writes every pending event before evget exits, including events still queued
between the input thread and storage:

- Linux: SIGINT (Ctrl+C), SIGTERM and SIGHUP. A `nohup` run ignores SIGHUP, as usual.
- Windows: Ctrl+C, Ctrl+Break, closing the console window, `taskkill /PID` or `taskkill /IM` without `/F`, and logging
  off or shutting down.

Windows never raises SIGTERM. A plain `taskkill` sends a close message to evget's hidden capture window instead. When
the console window is closed, Windows ends evget as soon as it has written its events, so evget exits with `0xC000013A`
(`STATUS_CONTROL_C_EXIT`) rather than 0. The queue is drained for at most 2 seconds, which keeps the write inside the
time Windows allows at console close and logoff; events still queued after that are dropped, and evget logs an error.

`taskkill` stops an elevated evget only when it is run from an elevated shell. From an unelevated shell, it fails with
`ERROR: The process with PID <pid> could not be terminated.` and evget keeps running.

A request that cannot be caught, such as `taskkill /F`, `Stop-Process`, ending the process in Task Manager, a crash or
SIGKILL, ends evget at once. The events that were not yet written are lost, which is at most the current `-n` / `-s`
batch and any events still queued. SQLite's write-ahead log keeps the file consistent, so it still passes
`pragma integrity_check` after a forced termination.

### Logs

Logs are written to standard output. To keep the logs of a hidden run, redirect standard output in the shell, for
example with `Start-Process -RedirectStandardOutput evget.log`, with `> evget.log`, or through journald for a systemd
unit. Combining `-RedirectStandardOutput` with `-WindowStyle Hidden` has not been tested. With `-o -`, events are
written to standard output and logging is turned off.

[x11-api]: https://www.x.org/releases/X11R7.6/doc/
[raw-input-header]: https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputheader
