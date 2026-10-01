# Integration tests

The integration tests are defined so that evget can be tested using synthetic inputs like a user would. They cover
input scenarios like mouse movement, keys, storing events, batching and others.

> [!WARNING]
> These tests inject real input into the running desktop. Don't use the machine while they run.

## Running

The `integration` recipe builds evget and runs the suite with [uv](https://docs.astral.sh/uv/), which
Arguments after the recipe go to pytest:

```sh
just integration
just linux::gcc::integration -k x11 -x
```

To test an existing binary, pass it with `--evget` or the `EVGET` environment variable:

```sh
uv run --group integration pytest tests/integration --evget build/linux-gcc-debug/bin/evget
```

Tests run once per backend. A backend that the host or the binary doesn't support is skipped.

## Requirements

- **Windows:** an interactive desktop session.
- **X11:** normally a valid display is required, but headless works under Xvfb, for example
  `xvfb-run -a just linux::integration`.
- **libinput:** read access to `/dev/input` and write access to `/dev/uinput`, which usually means root. The tests add
  a virtual uinput keyboard and mouse for the run.

Only the test's own input is counted. The evget process attributes this with `windows-injected`, `x11-injected` or the
virtual uinput device, meaning that other input on the desktop shouldn't change the results.
