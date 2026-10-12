"""
Tests that evget stores events correctly for the synthetic input.
"""

from __future__ import annotations

import sys
from collections import Counter
from pathlib import Path

import pytest

import driver
import stored

pytestmark = pytest.mark.integration

KEYS = 3
MOVES = 4
SCROLLS = 2


def capture(
    launch,
    backend: driver.Input,
    output: str,
    directory: Path,
    *args: str,
    **counts: int,
) -> stored.Tables:
    """
    Run evget while injecting input.
    """

    to_stdout = output == "-"
    process = launch(
        backend,
        output if to_stdout else directory / output,
        *driver.STORE_ON_STOP,
        *args,
    )

    backend.send(**counts)
    process.stop()
    assert process.wait() == 0

    if to_stdout:
        return stored.read_json(process.output())

    return stored.read(directory / output)


@pytest.mark.parametrize("output", ["events.sqlite", "events.json", "-"])
def test_injected_input_is_stored(
    launch, backend: driver.Input, tmp_path: Path, output: str
) -> None:
    # Reading "-" as JSON also checks that evget wrote nothing else to stdout.
    tables = capture(
        launch, backend, output, tmp_path, keys=KEYS, moves=MOVES, scrolls=SCROLLS
    )

    keys = stored.from_device(tables["key"], backend.device_name)
    actions = Counter(row["button_action"] for row in keys)
    assert actions == {"Press": KEYS, "Release": KEYS}
    assert {row["device_type"] for row in keys} == {"Keyboard"}

    moves = stored.from_device(tables["mouse_move"], backend.device_name)
    assert len(moves) == MOVES
    assert {row["device_type"] for row in moves} == {"Mouse"}

    scrolls = stored.from_device(tables["mouse_scroll"], backend.device_name)
    assert len(scrolls) == SCROLLS
    assert sum(float(row["scroll_vertical"]) for row in scrolls) == 0

    # Positive is down, and the driver scrolls up first.
    in_order = sorted(scrolls, key=lambda row: row["timestamp"])
    ups = [float(row["scroll_vertical"]) < 0 for row in in_order]
    assert ups == [i % 2 == 0 for i in range(SCROLLS)]

    for rows in tables.values():
        for row in rows:
            assert "" not in row.values()


def test_filter_stores_correct_device_types(
    launch, backend: driver.Input, tmp_path: Path
) -> None:
    tables = capture(
        launch,
        backend,
        "events.sqlite",
        tmp_path,
        "--filter",
        "keyboard",
        keys=KEYS,
        moves=MOVES,
        scrolls=SCROLLS,
    )

    assert len(stored.from_device(tables["key"], backend.device_name)) == 2 * KEYS

    for table in ("mouse_move", "mouse_scroll", "mouse_click"):
        assert stored.from_device(tables[table], backend.device_name) == []


@pytest.mark.parametrize("tap_to_click", [True, False], ids=["tap", "no-tap"])
def test_tap_to_click(
    launch,
    backend: driver.Input,
    touchpad: driver.LibinputTouchpad,
    tmp_path: Path,
    tap_to_click: bool,
) -> None:
    args = [] if tap_to_click else ["--no-tap-to-click"]
    database = tmp_path / "events.sqlite"
    process = launch(backend, database, *driver.STORE_ON_STOP, *args)
    touchpad.tap()
    process.stop()
    assert process.wait() == 0

    clicks = [
        row
        for row in stored.from_device(
            stored.read(database)["mouse_click"], backend.device_name
        )
        if row["device_type"] == "Touchpad"
    ]
    actions = Counter(row["button_action"] for row in clicks)
    assert actions == ({"Press": 1, "Release": 1} if tap_to_click else {})
    # A one-finger tap is the left button, stored with its evdev code as on every backend.
    assert {row["button_id"] for row in clicks} <= {driver.ecodes.BTN_LEFT}


@pytest.mark.parametrize("exclude", [False, True], ids=["title", "no-title"])
def test_exclude_window_title(
    launch, backend: driver.Input, tmp_path: Path, exclude: bool
) -> None:
    if not backend.records_focus_window:
        pytest.skip(f"{backend.backend} does not record the focused window")

    args = ["--exclude-window-title"] if exclude else []
    tables = capture(launch, backend, "events.sqlite", tmp_path, *args, keys=KEYS)

    keys = stored.from_device(tables["key"], backend.device_name)
    assert keys

    for row in keys:
        assert (row["focus_window_name"] is None) == exclude
        for field in ("position_x", "position_y", "width", "height"):
            assert row[f"focus_window_{field}"] is not None


@pytest.mark.skipif(
    sys.platform != "win32", reason="evget has a capture window only on Windows"
)
def test_capture_window_stays_out_of_taskbar(
    launch, backend: driver.Input, tmp_path: Path
) -> None:
    process = launch(backend, tmp_path / "events.sqlite")
    assert driver.taskbar_windows(process.pid) == []
