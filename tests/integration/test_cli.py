"""
Tests the command-line parsing that doesn't need input.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import pytest

pytestmark = pytest.mark.integration

# This should not be possible on the target platform.
FOREIGN_SOURCE = "x11" if sys.platform == "win32" else "windows"


def run(evget: Path, *args: str) -> subprocess.CompletedProcess[str]:
    """
    Runs the process.
    """

    return subprocess.run(
        [str(evget), *args], capture_output=True, text=True, timeout=30
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
    "args",
    [
        ["--no-such-option"],
        ["--store-n-events", "0"],
        ["--store-after-seconds", "-1"],
        ["--event-source", "keyboard"],
        ["--filter", "joystick"],
        ["--event-source", FOREIGN_SOURCE],
    ],
    ids=" ".join,
)
def test_invalid_arguments_exit_non_zero(
    evget: Path, tmp_path: Path, args: list[str]
) -> None:
    result = run(evget, *args, "--output", str(tmp_path / "events.sqlite"))
    assert result.returncode != 0
