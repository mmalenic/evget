"""
Tests the command-line parsing that doesn't need input.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import pytest

import driver

pytestmark = pytest.mark.integration

# This should not be possible on the target platform.
FOREIGN_SOURCE = "x11" if sys.platform == "win32" else "windows"


def run(evget: Path, *args: str) -> subprocess.CompletedProcess[str]:
    """
    Runs the process.
    """

    return subprocess.run(
        [str(evget), *args],
        capture_output=True,
        text=True,
        timeout=driver.scaled(30.0),
    )


def test_version_exits_zero(evget: Path) -> None:
    result = run(evget, "--version")
    assert result.returncode == 0
    assert result.stdout.startswith("evget ")


def test_help_exits_zero(evget: Path) -> None:
    result = run(evget, "--help")
    assert result.returncode == 0
    assert "--output" in result.stdout


@pytest.mark.parametrize(
    ("args", "exit_code"),
    [
        (["--no-such-option"], 109),
        (["--store-n-events", "0"], 105),
        (["--store-after-seconds", "-1"], 105),
        (["--event-source", "keyboard"], 104),
        (["--filter", "joystick"], 105),
    ],
    ids=lambda value: " ".join(value) if isinstance(value, list) else str(value),
)
def test_invalid_arguments_fail(
    evget: Path, tmp_path: Path, args: list[str], exit_code: int
) -> None:
    result = run(evget, *args, "--output", str(tmp_path / "events.sqlite"))
    assert result.returncode == exit_code, result.stderr
    assert args[0] in result.stderr


def test_foreign_event_source_fails(evget: Path, tmp_path: Path) -> None:
    output = str(tmp_path / "events.sqlite")
    result = run(evget, "--event-source", FOREIGN_SOURCE, "--output", output)
    assert result.returncode == 1, result.stdout + result.stderr
    assert driver.MISSING_BACKEND in result.stdout
