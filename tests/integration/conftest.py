"""
Options and config for the evget integration tests.
"""

from __future__ import annotations

import argparse
import os
import time
from collections.abc import Callable, Generator, Iterator
from pathlib import Path
from typing import NoReturn

import pytest

import driver

STDOUT_SETTLE = driver.scaled(2.0)
MISSING_BACKEND = "does not include the requested event source"

REPORTS = pytest.StashKey[dict[str, pytest.TestReport]]()


def backend_list(value: str) -> frozenset[str]:
    """
    Parse a comma-separated list of backends.
    """
    backends = frozenset(name.strip() for name in value.split(",") if name.strip())
    if unknown := backends - set(driver.BACKENDS):
        raise argparse.ArgumentTypeError(
            f"unknown backends: {','.join(sorted(unknown))}"
        )
    return backends


def pytest_addoption(parser: pytest.Parser) -> None:
    parser.addoption(
        "--evget",
        metavar="PATH",
        default=os.environ.get("EVGET"),
        help="the evget binary to test, defaults to $EVGET",
    )
    parser.addoption(
        "--require-backends",
        metavar="LIST",
        type=backend_list,
        default=os.environ.get("EVGET_REQUIRE_BACKENDS", ""),
        help="comma-separated backends that fail instead of skip when unavailable, "
        "defaults to $EVGET_REQUIRE_BACKENDS",
    )


def pytest_collection_modifyitems(
    config: pytest.Config, items: list[pytest.Item]
) -> None:
    # A timeout marker overrides --timeout, so scale only the configured default.
    if driver.TIMEOUT_SCALE == 1 or config.getoption("timeout", None) is not None:
        return
    if "PYTEST_TIMEOUT" not in os.environ:
        timeout = driver.scaled(float(config.getini("timeout")))
        for item in items:
            item.add_marker(pytest.mark.timeout(timeout))


@pytest.hookimpl(wrapper=True, tryfirst=True)
def pytest_runtest_makereport(
    item: pytest.Item,
) -> Generator[None, pytest.TestReport, pytest.TestReport]:
    report = yield
    item.stash.setdefault(REPORTS, {})[report.when] = report
    return report


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

    name = request.param
    try:
        device = driver.open_input(name)
    except driver.Unavailable as error:
        unavailable(request.config, name, str(error))
    try:
        if reason := missing_from_build(evget, device, tmp_path_factory.mktemp(name)):
            unavailable(request.config, name, reason)
        yield device
    finally:
        device.close()


def unavailable(config: pytest.Config, backend: str, reason: str) -> NoReturn:
    """
    Skip `backend`, or fail if --require-backends sets it.
    """

    if backend in config.getoption("require_backends"):
        message = f"required backend {backend} is unavailable: {reason}"
        raise pytest.fail.Exception(message, pytrace=False) from None

    pytest.skip(reason)


def missing_from_build(
    evget: Path, backend: driver.Input, directory: Path
) -> str | None:
    """
    Determines the reason why evget cannot run `backend`.
    """

    probe = driver.start(command(evget, backend, directory / "probe.json"), directory)
    try:
        probe.wait_ready()
    except driver.Exited as exited:
        if MISSING_BACKEND in exited.output:
            return f"{evget.name} was built without the {backend.backend} backend"
        raise
    except TimeoutError as error:
        raise TimeoutError(f"{error}\n{probe.log_tail()}") from None
    finally:
        probe.kill()
    return None


Launch = Callable[..., driver.Evget]


@pytest.fixture
def launch(
    request: pytest.FixtureRequest, evget: Path, tmp_path: Path
) -> Iterator[Launch]:
    """
    Start evget on a backend and wait until it captures input. Every evget started is killed when the
    test ends, and if the test failed, the end of its output is printed with the failure.
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
    call = request.node.stash.get(REPORTS, {}).get("call")

    if call is not None and call.failed:
        print(f"The last {driver.LOG_TAIL} lines of each evget log:")
        for process in started:
            print(process.log_tail())
