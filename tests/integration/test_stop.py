"""
Tests stop conditions for evget.
"""

from __future__ import annotations

import sys
import time
from pathlib import Path

import pytest

import driver
import stored

pytestmark = pytest.mark.integration

KEYS = 3


@pytest.mark.parametrize("stop", driver.STOP_METHODS)
def test_stop_writes_waiting_events(
    launch, backend: driver.Input, tmp_path: Path, stop: str
) -> None:
    database = tmp_path / "events.sqlite"
    process = launch(backend, database, *driver.STORE_ON_STOP, stop=stop)
    backend.send(keys=KEYS)

    process.stop()
    exit_code = process.wait()

    # Windows itself ends a process once its console-close handler returns.
    if stop != "console-close":
        assert exit_code == 0

    assert stored.count(database, "key", backend.device_name) == 2 * KEYS


def test_stop_without_input(
    launch, backend: driver.Input, tmp_path: Path
) -> None:
    process = launch(backend, tmp_path / "events.sqlite")

    started = time.monotonic()
    process.stop()
    assert process.wait() == 0

    assert time.monotonic() - started < 2.0


def test_kill_leaves_valid_database(
    launch, backend: driver.Input, tmp_path: Path
) -> None:
    database = tmp_path / "events.sqlite"
    process = launch(backend, database, "--store-n-events", "1")
    backend.send(keys=KEYS)

    process.kill()

    assert stored.integrity(database) == "ok"


@pytest.mark.skipif(sys.platform == "win32", reason="nohup and hangups are POSIX")
def test_nohup_ignores_hangup(launch, backend: driver.Input, tmp_path: Path) -> None:
    database = tmp_path / "events.sqlite"
    process = launch(backend, database, *driver.STORE_ON_STOP, stop="nohup")

    process.hang_up()
    with pytest.raises(TimeoutError):
        process.wait(timeout=1.0)
    backend.send(keys=KEYS)
    process.terminate()

    assert process.wait() == 0
    assert stored.count(database, "key", backend.device_name) == 2 * KEYS
