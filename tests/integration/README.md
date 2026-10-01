# Integration tests

The integration tests are defined so that evget can be tested using synthetic inputs like a user would. They cover
input scenarios like mouse movement, keys, storing events, batching and others.

> [!WARNING]
> These tests inject real input into the running desktop. Don't use the machine while they run.

## Running

The `integration` recipe builds evget and runs the suite with [uv](https://docs.astral.sh/uv/), which
installs the locked test dependencies into `.venv`. Without uv, it runs `python -m pytest`, so install the
dependencies first, e.g. in a virtual environment:

```sh
pip install --group integration
```

Arguments after the recipe go to pytest:

```sh
just integration
just linux::gcc::integration -k x11 -x
```

To test an existing binary, pass it with `--evget` or the `EVGET` environment variable:

```sh
uv run --locked --group integration pytest tests/integration --evget build/linux-gcc-debug/bin/evget
python -m pytest tests/integration --evget build/linux-gcc-debug/bin/evget   # without uv
```

Tests run once per backend. A backend that the host or the binary doesn't support is skipped.

## Requirements

- **Windows:** an interactive desktop session.
- **X11:** normally a valid display is required, but headless works under Xvfb.
- **libinput:** write access to `/dev/uinput` and read access to `/dev/input`, which usually means root. The tests add
  a virtual uinput keyboard and mouse for the run.

Only the test's own input is counted. The evget process attributes this with `windows-injected`, `x11-injected` or the
virtual uinput device, meaning that other input on the desktop shouldn't change the results.

## Headless and CI

On X11, headless can be ran under Xvfb:

```sh
xvfb-run -a just linux::integration --require-backends x11
```

On libinput, headless works directly with pytest run as root as udev creates the virtual device. Use `-E` to keep
variables such as `EVGET_TIMEOUT_SCALE`:

```sh
sudo modprobe uinput
just linux::build
uv sync --locked --group integration
sudo -E .venv/bin/pytest tests/integration --evget build/linux-gcc-debug/bin/evget --require-backends libinput
```

On Windows, an interactive desktop session is required, which GitHub-hosted runners have:

```sh
just windows::msvc::integration --require-backends windows
```

Options and variables for CI:

- `--require-backends LIST` or `EVGET_REQUIRE_BACKENDS=LIST`: a comma-separated list of backends that fail
  the run when unavailable, instead of being skipped.
- `--junitxml=build/integration.xml`: a JUnit report.
- `--basetemp=build/integration`: where each test keeps its evget databases and logs, to upload as
  artifacts. Only failed tests are kept.
- `EVGET_TIMEOUT_SCALE=3`: scales every wait for slow runners.
