"""
Tests that evget writes buffered events correctly.
"""

from __future__ import annotations

from pathlib import Path

import pytest

import driver
import stored

pytestmark = pytest.mark.integration

STORE_TIMEOUT = 10.0


def test_store_n_events_every_n_events(
    launch, backend: driver.Input, tmp_path: Path
) -> None:
    database = tmp_path / "events.sqlite"
    n, keys = 4, 4
    args = ("--store-n-events", str(n), "--store-after-seconds", "3600")
    process = launch(backend, database, *args)

    backend.send(keys=keys)

    # Other input on the desktop counts towards n too, leaving up to n - 1.
    driver.wait_until(
        lambda: stored.count(database, "key", backend.device_name) >= 2 * keys - n + 1,
        STORE_TIMEOUT,
        "key rows to be stored",
    )

    with pytest.raises(TimeoutError):
        process.wait(timeout=0)


def test_store_after_seconds_writes_on_interval(
    launch, backend: driver.Input, tmp_path: Path
) -> None:
    database = tmp_path / "events.sqlite"
    args = ("--store-n-events", "100000", "--store-after-seconds", "1")
    process = launch(backend, database, *args)

    backend.send(keys=1)

    driver.wait_until(
        lambda: stored.count(database, "key", backend.device_name) == 2,
        STORE_TIMEOUT,
        "key rows to be stored",
    )

    with pytest.raises(TimeoutError):
        process.wait(timeout=0)
