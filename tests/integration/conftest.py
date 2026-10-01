"""
Options and config for the evget integration tests.
"""

from __future__ import annotations

import os
import time
from collections.abc import Callable, Iterator
from pathlib import Path

import pytest

import driver

STDOUT_SETTLE = 2.0
MISSING_BACKEND = "does not include the requested event source"

def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption(
        "--evget",
        metavar="PATH",
        default=os.environ.get("EVGET"),
        help="the evget binary to test, defaults to $EVGET",
    )


@pytest.fixture(scope="session")
def evget(pytestconfig: pytest.Config) -> Path:
    """
    The evget binary
    """

    option = pytestconfig.getoption("evget")
    if not option:
        pytest.skip("no evget binary to test: pass --evget PATH or set EVGET")
    path = Path(option).resolve()
    if not path.is_file():
        pytest.exit(f"evget binary not found: {path}", pytest.ExitCode.USAGE_ERROR)
    return path


def command(
    evget: Path, backend: driver.Input, output: Path | str, *args: str
) -> list[str]:
    """
    The command line that runs evget on `backend`.
    """

    return [
        str(evget),
        "--event-source",
        backend.backend,
        *backend.args,
        "--log-level",
        "info",
        "--output",
        str(output),
        *args,
    ]


@pytest.fixture(scope="session", params=driver.BACKENDS)
def backend(
    request: pytest.FixtureRequest,
    evget: Path,
    tmp_path_factory: pytest.TempPathFactory,
) -> Iterator[driver.Input]:
    """
    The synthetic input for each backend.
    """

    if reason := driver.unavailable(request.param):
        pytest.skip(reason)
    device = driver.open_input(request.param)
    try:
        require_backend(evget, device, tmp_path_factory.mktemp(request.param))
        yield device
    finally:
        device.close()


def require_backend(evget: Path, backend: driver.Input, directory: Path) -> None:
    """
    Specifies the required `backend` for evget to be built with.
    """

    probe = driver.start(command(evget, backend, directory / "probe.json"), directory)
    try:
        probe.wait_ready()
    except driver.Exited as exited:
        if MISSING_BACKEND in exited.output:
            pytest.skip(f"{evget.name} was built without the {backend.backend} backend")
        raise
    finally:
        probe.kill()


Launch = Callable[..., driver.Evget]


@pytest.fixture
def launch(evget: Path, tmp_path: Path) -> Iterator[Launch]:
    """
    Start evget on a backend and wait until it captures input. Every evget started is killed when the
    test ends, whether it passed or not.
    """

    started: list[driver.Evget] = []

    def start(
        backend: driver.Input,
        output: Path | str,
        *args: str,
        stop: str = driver.TERMINATE,
    ) -> driver.Evget:
        process = driver.start(command(evget, backend, output, *args), tmp_path, stop)
        started.append(process)
        if output == "-":
            time.sleep(STDOUT_SETTLE)
        else:
            process.wait_ready()
        return process

    yield start
    for process in started:
        process.kill()
