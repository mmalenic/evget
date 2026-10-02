"""
The input and evget process driver that creates synthetic input. On Windows this file
also runs as the helper for `Console` starts.
"""

from __future__ import annotations

import contextlib
import ctypes
import functools
import os
import signal
import subprocess
import sys
import time
from abc import ABC, abstractmethod
from collections.abc import Callable
from pathlib import Path
from typing import TypeVar, cast

import psutil

if sys.platform == "win32":
    import win32api
    import win32con
    import win32console
    import win32gui
    import win32job
    import win32process
else:
    import pexpect

if sys.platform == "linux":
    import evdev
    from evdev import ecodes
    from Xlib import X
    from Xlib.display import Display
    from Xlib.error import DisplayError
    from Xlib.ext import xtest

BACKENDS = ("windows", "x11", "libinput")

READY = "capturing input events"

TIMEOUT_SCALE = float(os.environ.get("EVGET_TIMEOUT_SCALE", "1"))


def scaled(seconds: float) -> float:
    """
    `seconds` scaled by `EVGET_TIMEOUT_SCALE`.
    """
    return seconds * TIMEOUT_SCALE


READY_TIMEOUT = scaled(15.0)
STOP_TIMEOUT = scaled(10.0)
DELIVERY_DELAY = scaled(0.5)
OPEN_SETTLE = scaled(0.5)
PACE = 0.01
MOVE_STEP = 5
LOG_TAIL = 30

STORE_ON_STOP = ("--store-n-events", "100000", "--store-after-seconds", "3600")

if sys.platform == "win32":
    TERMINATE = "taskkill"
    STOP_METHODS = (
        "taskkill",
        "taskkill-image",
        "ctrl-c",
        "ctrl-break",
        "console-close",
    )
else:
    TERMINATE = "sigterm"
    STOP_METHODS = ("sigterm", "ctrl-c", "hangup")

T = TypeVar("T")

# <linux/prctl.h>; Python exports no name for it.
PR_SET_PDEATHSIG = 1

# A timeout may end pytest by os._exit, skipping teardown, so the OS reaps children.
DIE_WITH_PARENT: Callable[[], None] | None = None
"""A `preexec_fn` that kills the child once the test run dies, however it dies."""
if sys.platform == "linux":
    DIE_WITH_PARENT = functools.partial(
        ctypes.CDLL(None).prctl, PR_SET_PDEATHSIG, signal.SIGKILL
    )


@functools.cache
def _job() -> int:
    """A job that kills every process in it once this process exits."""
    # The stubs type the returned PyHANDLE as None.
    job = cast(int, win32job.CreateJobObject(None, ""))
    info_class = win32job.JobObjectExtendedLimitInformation
    limits = win32job.QueryInformationJobObject(job, info_class)
    limits["BasicLimitInformation"]["LimitFlags"] |= (
        win32job.JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    )
    win32job.SetInformationJobObject(job, info_class, limits)
    return job


def _contain(pid: int) -> None:
    """Make process `pid`, and every process it starts, die with this process."""
    access = win32con.PROCESS_SET_QUOTA | win32con.PROCESS_TERMINATE
    handle = win32api.OpenProcess(access, False, pid)
    try:
        win32job.AssignProcessToJobObject(_job(), handle)
    finally:
        win32api.CloseHandle(handle)


def wait_until(condition: Callable[[], T | None], timeout: float, what: str) -> T:
    """
    Poll `condition` until it returns a truthy value.
    """
    deadline = time.monotonic() + timeout
    while not (result := condition()):
        if time.monotonic() > deadline:
            raise TimeoutError(f"timed out after {timeout:g} s waiting for {what}")
        time.sleep(0.05)

    return result


class Unavailable(Exception):
    """
    tHE host cannot run a backend.
    """


class Input(ABC):
    """
    Synthetic input that evget attributes to `device_name`.
    """

    backend: str
    device_name: str
    args: tuple[str, ...] = ()
    records_focus_window = True

    @abstractmethod
    def key(self, down: bool) -> None:
        """
        Press or release F24.
        """

    @abstractmethod
    def move(self, dx: int, dy: int) -> None:
        """
        Move the pointer relative to where it is.
        """

    @abstractmethod
    def scroll(self, up: bool) -> None:
        """
        Turn the wheel one increment.
        """

    @abstractmethod
    def opened_by(self, pid: int) -> bool:
        """Whether process `pid` has opened this input to capture it."""

    def close(self) -> None:
        """
        Release the input device.
        """

    def send(self, keys: int = 0, moves: int = 0, scrolls: int = 0) -> None:
        """
        Press F24, then move and scroll. This is intended to leave the pointer
        and scroll at the original position.
        """

        for _ in range(keys):
            self.key(down=True)
            time.sleep(PACE)
            self.key(down=False)
            time.sleep(PACE)

        for i in range(moves):
            step = MOVE_STEP if i % 2 == 0 else -MOVE_STEP
            self.move(step, step)
            time.sleep(PACE)

        for i in range(scrolls):
            self.scroll(up=i % 2 == 0)
            time.sleep(PACE)

        time.sleep(DELIVERY_DELAY)


class WindowsInput(Input):
    """
    The Windows Injected input.
    """

    backend = "windows"
    device_name = "windows-injected"

    def key(self, down: bool) -> None:
        flags = 0 if down else win32con.KEYEVENTF_KEYUP
        win32api.keybd_event(win32con.VK_F24, 0, flags)

    def move(self, dx: int, dy: int) -> None:
        win32api.mouse_event(win32con.MOUSEEVENTF_MOVE, dx, dy)

    def scroll(self, up: bool) -> None:
        delta = win32con.WHEEL_DELTA if up else -win32con.WHEEL_DELTA
        win32api.mouse_event(win32con.MOUSEEVENTF_WHEEL, 0, 0, delta)

    def opened_by(self, pid: int) -> bool:
        # evget registers for Raw Input as soon as it has created its capture window.
        return bool(windows_of(pid))


class X11Input(Input):
    """
    The XTEST injected input.
    """

    backend = "x11"
    device_name = "x11-injected"

    def __init__(self) -> None:
        # X servers number keys by kernel code plus 8.
        self._f24 = ecodes.KEY_F24 + 8
        try:
            self._display = Display()
        except DisplayError as error:
            raise Unavailable(
                f"no X display ({error}), set DISPLAY or run under xvfb-run"
            ) from error
        self._window = self._focus_window()

    def key(self, down: bool) -> None:
        event = X.KeyPress if down else X.KeyRelease
        xtest.fake_input(self._display, event, self._f24)
        self._display.sync()

    def move(self, dx: int, dy: int) -> None:
        xtest.fake_input(self._display, X.MotionNotify, detail=True, x=dx, y=dy)
        self._display.sync()

    def scroll(self, up: bool) -> None:
        button = X.Button4 if up else X.Button5
        xtest.fake_input(self._display, X.ButtonPress, button)
        xtest.fake_input(self._display, X.ButtonRelease, button)
        self._display.sync()

    def opened_by(self, pid: int) -> bool:
        # The display connection is the only socket evget opens.
        return any(target.startswith("socket:") for target in _open_files(pid))

    def close(self) -> None:
        if self._window is not None:
            self._window.destroy()
        self._display.close()

    def _focus_window(self):
        """
        The focus window if there is one.
        """

        if not isinstance(self._display.get_input_focus().focus, int):
            return None

        screen = self._display.screen()
        window = screen.root.create_window(0, 0, 320, 240, 0, screen.root_depth)
        window.set_wm_name("evget integration tests")
        window.map()
        window.set_input_focus(X.RevertToParent, X.CurrentTime)
        self._display.sync()

        return window


if sys.platform == "linux":

    class _UInput(evdev.UInput):
        """
        A uinput device that removes itself if it cannot open the event node.
        """

        def _find_device(self, fd: int) -> evdev.InputDevice:
            try:
                return super()._find_device(fd)
            except OSError:
                # Closing the uinput descriptor removes the device UInput created.
                os.close(fd)
                raise


class LibinputInput(Input):
    """
    The libinput virtual uinput keyboard and mouse for synthetic events.
    """

    backend = "libinput"
    device_name = "evget integration tests"
    args = ("--screen-dimensions", "1920x1080")
    records_focus_window = False

    def __init__(self) -> None:
        capabilities = {
            ecodes.EV_KEY: [
                ecodes.KEY_F24,
                ecodes.BTN_LEFT,
                ecodes.BTN_RIGHT,
                ecodes.BTN_MIDDLE,
            ],
            ecodes.EV_REL: [ecodes.REL_X, ecodes.REL_Y, ecodes.REL_WHEEL],
        }
        try:
            self._device = _UInput(capabilities, name=self.device_name)
        except (evdev.UInputError, PermissionError) as error:
            raise Unavailable(
                f"{error}"
            ) from error

        # libinput skips a device until udev has written a database entry.
        rdev = os.stat(self._device.device.path).st_rdev

        entry = Path(f"/run/udev/data/c{os.major(rdev)}:{os.minor(rdev)}")
        try:
            wait_until(entry.exists, READY_TIMEOUT, f"udev to add {self.device_name!r}")
        except TimeoutError as error:
            self._device.close()
            raise Unavailable(f"{error}; is udevd running?") from error

    def key(self, down: bool) -> None:
        self._device.write(ecodes.EV_KEY, ecodes.KEY_F24, int(down))
        self._device.syn()

    def move(self, dx: int, dy: int) -> None:
        self._device.write(ecodes.EV_REL, ecodes.REL_X, dx)
        self._device.write(ecodes.EV_REL, ecodes.REL_Y, dy)
        self._device.syn()

    def scroll(self, up: bool) -> None:
        self._device.write(ecodes.EV_REL, ecodes.REL_WHEEL, 1 if up else -1)
        self._device.syn()

    def opened_by(self, pid: int) -> bool:
        return self._device.device.path in _open_files(pid)

    def close(self) -> None:
        self._device.close()


def open_input(backend: str) -> Input:
    """
    Create the synthetic input for `backend`.
    """
    if backend == "windows":
        if sys.platform != "win32":
            raise Unavailable("the windows backend needs Windows")
        return WindowsInput()
    if sys.platform != "linux":
        raise Unavailable(f"the {backend} backend needs Linux")

    return X11Input() if backend == "x11" else LibinputInput()


def start(command: list[str], directory: Path, stop: str = TERMINATE) -> Evget:
    """
    Start evget so that `Evget.stop` stops it automatically.
    """
    if sys.platform == "win32" and stop in ("ctrl-c", "ctrl-break", "console-close"):
        return Console(command, directory, stop)
    if sys.platform != "win32" and stop in ("ctrl-c", "hangup", "nohup"):
        return Terminal(command, directory, stop)

    return Process(command, directory, stop)


class Exited(Exception):
    """
    The evget process exited before it captured input.
    """

    def __init__(self, output: str) -> None:
        super().__init__(f"evget exited before it captured input:\n{output}")
        self.output = output


class Evget(ABC):
    """
    Run evget.
    """

    stdout: Path

    @property
    @abstractmethod
    def pid(self) -> int:
        """
        The evget process id.
        """

    @property
    def logs(self) -> list[Path]:
        """
        A list of files to the evget logs.
        """
        return [self.stdout]

    def wait_ready(self, timeout: float = READY_TIMEOUT) -> None:
        """
        Wait until evget logs that it captures input.
        """
        self._wait_running_until(
            lambda: READY in self.output(), timeout, "evget to capture input"
        )

    def wait_opened(self, backend: Input, timeout: float = READY_TIMEOUT) -> None:
        """Wait until evget captures `backend`'s input, for a run that logs nothing.

        Raises `Exited` if evget exits first.
        """
        self._wait_running_until(
            lambda: backend.opened_by(self.pid),
            timeout,
            f"evget to open the {backend.backend} input",
        )
        time.sleep(OPEN_SETTLE)

    def _wait_running_until(
        self, condition: Callable[[], bool], timeout: float, what: str
    ) -> None:
        """Poll `condition` like `wait_until`, raising `Exited` if evget exits."""

        def done() -> bool:
            if condition():
                return True
            try:
                self.wait(timeout=0)
            except TimeoutError:
                return False
            raise Exited(self.output())

        wait_until(done, timeout, what)

    @abstractmethod
    def stop(self) -> None:
        """
        Stop evget.
        """

    @abstractmethod
    def wait(self, timeout: float = STOP_TIMEOUT) -> int | None:
        """
        Wait for evget to exit and return the exit code.
        """

    @abstractmethod
    def kill(self) -> None:
        """
        Kill evget if still running.
        """

    def output(self) -> str:
        """
        The output of evget so far.
        """
        return _read(self.stdout)

    def log_tail(self, lines: int = LOG_TAIL) -> str:
        """
        The last `lines` of the evget logs.
        """
        return "\n".join(
            f"{log}:\n" + "\n".join(_read(log).splitlines()[-lines:])
            for log in self.logs
        )


class Process(Evget):
    """
    The evget process.
    """

    def __init__(self, command: list[str], directory: Path, stop: str) -> None:
        self._stop = stop
        self._image = Path(command[0]).name
        self.stdout = directory / "stdout"
        self.stderr = directory / "stderr"

        with self.stdout.open("wb") as stdout, self.stderr.open("wb") as stderr:
            self._process = subprocess.Popen(
                command,
                stdin=subprocess.DEVNULL,
                stdout=stdout,
                stderr=stderr,
                preexec_fn=DIE_WITH_PARENT,  # noqa: PLW1509 - the suite starts no threads
            )
        if sys.platform == "win32":
            _contain(self._process.pid)

    @property
    def pid(self) -> int:
        return self._process.pid

    @property
    def logs(self) -> list[Path]:
        return [self.stdout, self.stderr]

    def stop(self) -> None:
        if self._stop == "sigterm":
            self._process.terminate()
        elif self._stop == "taskkill":
            _taskkill("/PID", str(self.pid))
        elif self._stop == "taskkill-image":
            # The PID should leave any other evget the user is running.
            _taskkill("/IM", self._image, "/FI", f"PID eq {self.pid}")
        else:
            raise ValueError(f"a plain process cannot be stopped by {self._stop!r}")

    def wait(self, timeout: float = STOP_TIMEOUT) -> int | None:
        try:
            return self._process.wait(timeout)
        except subprocess.TimeoutExpired as error:
            raise TimeoutError(f"evget still running after {timeout:g} s") from error

    def kill(self) -> None:
        self._process.kill()
        self._process.wait()


class Terminal(Evget):
    """
    The evget in a pseudo-terminal.
    """

    def __init__(self, command: list[str], directory: Path, stop: str) -> None:
        self._stop = stop
        self._hung_up = False
        self._transcript = directory / "terminal"
        self.stdout = self._transcript
        if stop == "nohup":
            command = ["nohup", *command]
            # nohup redirects the output of a command run from a terminal here.
            self.stdout = directory / "nohup.out"
        self._child = pexpect.spawn(
            command[0],
            command[1:],
            cwd=directory,
            encoding="utf-8",
            preexec_fn=DIE_WITH_PARENT,
        )
        self._child.logfile_read = self._transcript.open("w", encoding="utf-8")

    @property
    def pid(self) -> int:
        return self._child.pid

    @property
    def logs(self) -> list[Path]:
        return list(dict.fromkeys([self.stdout, self._transcript]))

    def wait_ready(self, timeout: float = READY_TIMEOUT) -> None:
        if self.stdout != self._transcript:
            super().wait_ready(timeout)
            return
        try:
            self._child.expect_exact(READY, timeout=timeout)
        except pexpect.EOF as error:
            raise Exited(self.output()) from error
        except pexpect.TIMEOUT as error:
            raise TimeoutError(
                f"evget did not capture input in {timeout:g} s"
            ) from error

    def stop(self) -> None:
        if self._stop == "ctrl-c":
            self._child.sendintr()
        elif self._stop == "hangup":
            self.hang_up()
        else:
            self.terminate()

    def hang_up(self) -> None:
        """
        Close the termina.
        """

        # pexpect close() follows hangup with SIGINT and SIGKILL.
        self._child.ptyproc.fileobj.close()
        self._hung_up = True

    def terminate(self) -> None:
        os.kill(self.pid, signal.SIGTERM)

    def wait(self, timeout: float = STOP_TIMEOUT) -> int | None:
        deadline = time.monotonic() + timeout
        if not self._hung_up:
            # Reading the terminal to its end keeps the last lines.
            try:
                self._child.expect(pexpect.EOF, timeout=timeout)
            except pexpect.TIMEOUT as error:
                raise TimeoutError(
                    f"evget still running after {timeout:g} s"
                ) from error
        remaining = max(0.0, deadline - time.monotonic())
        wait_until(lambda: not self._child.isalive(), remaining, "evget to exit")
        return self._child.exitstatus

    def kill(self) -> None:
        self._child.close(force=True)
        self._child.logfile_read.close()


class Console(Evget):
    """
    A console for evget.
    """

    def __init__(self, command: list[str], directory: Path, stop: str) -> None:
        self._stop = stop
        self._image = Path(command[0]).name.lower()
        self.stdout = directory / "stdout"
        self._helper_log = directory / "helper"
        self._console = directory / "console-window"
        helper = [
            sys.executable,
            __file__,
            stop,
            str(self.stdout),
            str(self._console),
            str(self._helper_log),
            *command,
        ]
        
        if stop == "console-close":
            # conhost gives the helper a classic console window to close.
            self._helper = subprocess.Popen(["conhost.exe", *helper])
        else:
            with self._helper_log.open("wb") as log:
                self._helper = subprocess.Popen(
                    helper,
                    stdin=subprocess.PIPE,
                    stdout=subprocess.DEVNULL,
                    stderr=log,
                    creationflags=subprocess.CREATE_NEW_CONSOLE,
                    startupinfo=subprocess.STARTUPINFO(
                        dwFlags=subprocess.STARTF_USESHOWWINDOW,
                        wShowWindow=subprocess.SW_HIDE,
                    ),
                )
                
        try:
            _contain(self._helper.pid)
            self._evget = wait_until(
                self._find_evget, READY_TIMEOUT, "the console helper to start evget"
            )
        except BaseException:
            self._kill_all()
            raise

    @property
    def pid(self) -> int:
        return self._evget.pid

    @property
    def logs(self) -> list[Path]:
        return [self.stdout, self._helper_log]

    def stop(self) -> None:
        if self._stop == "console-close":
            # A console window names its client, not conhost, as its process.
            reported = wait_until(
                lambda: _read(self._console).strip(),
                READY_TIMEOUT,
                "the console helper to report its window",
            )
            window = int(reported)
            if not window:
                raise RuntimeError("the console helper has no console window")
            if (name := win32gui.GetClassName(window)) != "ConsoleWindowClass":
                raise RuntimeError(
                    f"the console window is a {name}, not a ConsoleWindowClass to close"
                )
            win32gui.PostMessage(window, win32con.WM_CLOSE, 0, 0)
        else:
            assert self._helper.stdin is not None
            self._helper.stdin.write(b"\n")
            self._helper.stdin.close()

    def wait(self, timeout: float = STOP_TIMEOUT) -> int | None:
        try:
            if self._stop == "console-close":
                return self._evget.wait(timeout)
            # The helper exits with the evget exit code.
            return self._helper.wait(timeout)
        except (subprocess.TimeoutExpired, psutil.TimeoutExpired) as error:
            raise TimeoutError(f"evget still running after {timeout:g} s") from error

    def kill(self) -> None:
        self._kill_all(self._evget)

    def _kill_all(self, *known: psutil.Process) -> None:
        """Kill the helper and everything it started, including `known` processes."""
        processes = {*known, *self._descendants()}
        for process in processes:
            with contextlib.suppress(psutil.NoSuchProcess):
                process.kill()
        psutil.wait_procs(processes, timeout=STOP_TIMEOUT)
        self._helper.kill()
        self._helper.wait()

    def _descendants(self) -> list[psutil.Process]:
        try:
            return psutil.Process(self._helper.pid).children(recursive=True)
        except psutil.NoSuchProcess:
            return []

    def _find_evget(self) -> psutil.Process | None:
        for process in self._descendants():
            with contextlib.suppress(psutil.NoSuchProcess):
                if process.name().lower() == self._image:
                    return process
        return None


def _read(path: Path) -> str:
    if not path.exists():
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def _open_files(pid: int) -> list[str]:
    """What the file descriptors of process `pid` point at, such as a device or socket."""
    targets: list[str] = []
    # An exited process has no descriptors to list, not even for its parent.
    with contextlib.suppress(OSError):
        for descriptor in Path(f"/proc/{pid}/fd").iterdir():
            with contextlib.suppress(OSError):
                targets.append(os.readlink(descriptor))
    return targets


def _taskkill(*args: str) -> None:
    try:
        subprocess.run(
            ["taskkill", *args],
            check=True,
            capture_output=True,
            encoding="oem",
            errors="replace",
        )
    except subprocess.CalledProcessError as error:
        raise RuntimeError(f"{error} {error.stderr.strip()}") from error


def windows_of(pid: int) -> list[int]:
    """
    The top-level windows of the process `pid`.
    """
    windows: list[int] = []

    def collect(window: int, _: None) -> bool:
        if win32process.GetWindowThreadProcessId(window)[1] == pid:
            windows.append(window)

        return True

    win32gui.EnumWindows(collect, None)
    return windows


def taskbar_windows(pid: int) -> list[int]:
    """
    The windows of process `pid` that the taskbar and Alt+Tab have.
    """

    def listed(window: int) -> bool:
        if not win32gui.IsWindowVisible(window):
            return False

        style = win32gui.GetWindowLong(window, win32con.GWL_EXSTYLE)
        if style & win32con.WS_EX_APPWINDOW:
            return True

        owner = win32gui.GetWindow(window, win32con.GW_OWNER)
        return not style & win32con.WS_EX_TOOLWINDOW and not owner

    return [window for window in windows_of(pid) if listed(window)]


def _console_helper(
    stop: str, stdout: str, console: str, log: str, *command: str
) -> int:
    """
    Run evget in this console.
    """
    
    sys.stderr = open(log, "a", encoding="utf-8", buffering=1)  # noqa: SIM115
    Path(console).write_text(str(win32console.GetConsoleWindow()))
    # Ctrl+Break reaches only the process group it is sent to.
    group = subprocess.CREATE_NEW_PROCESS_GROUP if stop == "ctrl-break" else 0
    with open(stdout, "wb") as output:
        process = subprocess.Popen(
            command,
            stdin=subprocess.DEVNULL,
            stdout=output,
            stderr=subprocess.STDOUT,
            creationflags=group,
        )

    if stop == "console-close":
        # Closing the window stops evget.
        return process.wait()
    if not sys.stdin.readline():
        # The test failed, so we stop evget.
        process.kill()
    elif stop == "ctrl-break":
        process.send_signal(signal.CTRL_BREAK_EVENT)
    else:
        # Ignoring Ctrl+C only after starting evget keeps evget from inheriting it.
        win32api.SetConsoleCtrlHandler(None, True)
        win32api.GenerateConsoleCtrlEvent(win32con.CTRL_C_EVENT, 0)
        
    return process.wait()


if __name__ == "__main__":
    sys.exit(_console_helper(*sys.argv[1:]))
