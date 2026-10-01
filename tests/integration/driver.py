"""
The input and evget process driver that creates synthetic input. On Windows this file
also runs as the helper for `Console` starts.
"""

from __future__ import annotations

import os
import signal
import subprocess
import sys
import time
from abc import ABC, abstractmethod
from collections.abc import Callable
from pathlib import Path
from typing import TypeVar

import psutil

if sys.platform == "win32":
    import win32api
    import win32con
    import win32gui
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

READY_TIMEOUT = 15.0
STOP_TIMEOUT = 10.0

STORE_ON_STOP = ("--store-n-events", "100000", "--store-after-seconds", "3600")

DELIVERY_DELAY = 0.5
PACE = 0.01
MOVE_STEP = 5

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


def wait_until(condition: Callable[[], T], timeout: float, what: str) -> T:
    """
    Poll `condition` until it returns a truthy.
    """

    deadline = time.monotonic() + timeout
    while not (result := condition()):
        if time.monotonic() > deadline:
            raise TimeoutError(f"timed out after {timeout:g} s waiting for {what}")
        time.sleep(0.05)

    return result


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


class X11Input(Input):
    """
    The XTEST injected input.
    """

    backend = "x11"
    device_name = "x11-injected"

    def __init__(self) -> None:
        # X servers number keys by kernel code plus 8.
        self._f24 = ecodes.KEY_F24 + 8
        self._display = Display()
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
        self._device = evdev.UInput(capabilities, name=self.device_name)

        # libinput skips a device until udev has written a database entry.
        rdev = os.stat(self._device.device.path).st_rdev

        entry = Path(f"/run/udev/data/c{os.major(rdev)}:{os.minor(rdev)}")
        wait_until(entry.exists, READY_TIMEOUT, f"udev to add {self.device_name!r}")

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

    def close(self) -> None:
        self._device.close()


def unavailable(backend: str) -> str | None:
    """
    The reason why `backend` cannot be tested on this host.\
    """

    if backend == "windows":
        return None if sys.platform == "win32" else "the windows backend needs Windows"
    if sys.platform != "linux":
        return f"the {backend} backend needs Linux"
    if backend == "x11":
        try:
            Display().close()
        except DisplayError as error:
            return f"no X display ({error}): set DISPLAY or run under xvfb-run"
        return None
    if not os.access("/dev/uinput", os.W_OK):
        return "libinput needs write access to /dev/uinput"
    if not any(os.access(node, os.R_OK) for node in Path("/dev/input").glob("event*")):
        return "libinput needs read access to /dev/input"

    return None


def open_input(backend: str) -> Input:
    """
    Create the synthetic input for `backend`.
    """

    inputs = {"windows": WindowsInput, "x11": X11Input, "libinput": LibinputInput}
    return inputs[backend]()


def start(command: list[str], directory: Path, stop: str = TERMINATE) -> Evget:
    """
    Start evget so that `Evget.stop` stops it automatically.
    """

    if sys.platform == "win32" and stop in ("ctrl-c", "console-close"):
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

    stdout: Path | None = None

    @property
    @abstractmethod
    def pid(self) -> int:
        """
        The evget process id.
        """

    def wait_ready(self, timeout: float = READY_TIMEOUT) -> None:
        """
        Wait until evget logs that it captures input.
        """

        def ready() -> bool:
            if READY in self.output():
                return True
            try:
                self.wait(timeout=0)
            except TimeoutError:
                return False
            raise Exited(self.output())

        wait_until(ready, timeout, "evget to capture input")

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
        What evget has written to standard output.
        """

        if self.stdout is None or not self.stdout.exists():
            return ""
        return self.stdout.read_text(encoding="utf-8", errors="replace")


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
                # Ctrl+Break only goes to the process group it is sent to.
                creationflags=subprocess.CREATE_NEW_PROCESS_GROUP
                if sys.platform == "win32"
                else 0,
            )

    @property
    def pid(self) -> int:
        return self._process.pid

    def stop(self) -> None:
        if self._stop == "sigterm":
            self._process.terminate()
        elif self._stop == "taskkill":
            _taskkill("/PID", str(self.pid))
        elif self._stop == "taskkill-image":
            # The PID should leave any other evget the user is running.
            _taskkill("/IM", self._image, "/FI", f"PID eq {self.pid}")
        elif self._stop == "ctrl-break":
            self._process.send_signal(signal.CTRL_BREAK_EVENT)
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
        if stop == "nohup":
            command = ["nohup", *command]
            # nohup redirects the output of a command run from a terminal here.
            self.stdout = directory / "nohup.out"
        self._child = pexpect.spawn(
            command[0], command[1:], cwd=directory, encoding="utf-8"
        )

    @property
    def pid(self) -> int:
        return self._child.pid

    def wait_ready(self, timeout: float = READY_TIMEOUT) -> None:
        if self.stdout is not None:
            super().wait_ready(timeout)
            return
        try:
            self._child.expect_exact(READY, timeout=timeout)
        except pexpect.EOF as error:
            raise Exited(self._child.before) from error
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

    def terminate(self) -> None:
        os.kill(self.pid, signal.SIGTERM)

    def wait(self, timeout: float = STOP_TIMEOUT) -> int | None:
        wait_until(lambda: not self._child.isalive(), timeout, "evget to exit")
        return self._child.exitstatus

    def kill(self) -> None:
        self._child.close(force=True)


class Console(Evget):
    """
    A console for evget.
    """

    def __init__(self, command: list[str], directory: Path, stop: str) -> None:
        self._stop = stop
        self._image = Path(command[0]).name.lower()
        self.stdout = directory / "stdout"
        helper = [sys.executable, __file__, str(self.stdout), *command]
        if stop == "console-close":
            # conhost gives the helper a classic console window to close.
            self._helper = subprocess.Popen(["conhost.exe", *helper])
        else:
            self._helper = subprocess.Popen(
                helper,
                stdin=subprocess.PIPE,
                creationflags=subprocess.CREATE_NEW_CONSOLE,
                startupinfo=subprocess.STARTUPINFO(
                    dwFlags=subprocess.STARTF_USESHOWWINDOW,
                    wShowWindow=subprocess.SW_HIDE,
                ),
            )
        self._evget = wait_until(
            self._find_evget, READY_TIMEOUT, "the console helper to start evget"
        )

    @property
    def pid(self) -> int:
        return self._evget.pid

    def stop(self) -> None:
        if self._stop == "ctrl-c":
            self._helper.stdin.write(b"\n")
            self._helper.stdin.close()
        else:
            window = wait_until(
                self._console_window, READY_TIMEOUT, "the conhost console window"
            )
            win32gui.PostMessage(window, win32con.WM_CLOSE, 0, 0)

    def wait(self, timeout: float = STOP_TIMEOUT) -> int | None:
        try:
            # The helper exits with the evget exit code.
            if self._stop == "ctrl-c":
                return self._helper.wait(timeout)
            return self._evget.wait(timeout)
        except (subprocess.TimeoutExpired, psutil.TimeoutExpired) as error:
            raise TimeoutError(f"evget still running after {timeout:g} s") from error

    def kill(self) -> None:
        for process in [*self._descendants(), psutil.Process(self._helper.pid)]:
            try:
                process.kill()
            except psutil.NoSuchProcess:
                pass

        self._helper.wait()

    def _descendants(self) -> list[psutil.Process]:
        try:
            return psutil.Process(self._helper.pid).children(recursive=True)
        except psutil.NoSuchProcess:
            return []

    def _find_evget(self) -> psutil.Process | None:
        for process in self._descendants():
            try:
                if process.name().lower() == self._image:
                    return process
            except psutil.NoSuchProcess:
                pass

        return None

    def _console_window(self) -> int | None:
        windows = windows_of(self._helper.pid)
        return next(
            (w for w in windows if win32gui.GetClassName(w) == "ConsoleWindowClass"),
            None,
        )


def _taskkill(*args: str) -> None:
    subprocess.run(["taskkill", *args], check=True, capture_output=True)


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


def _console_helper(stdout: str, *command: str) -> int:
    """
    Run  evget in this console
    """

    with open(stdout, "wb") as output:
        process = subprocess.Popen(
            command,
            stdin=subprocess.DEVNULL,
            stdout=output,
            stderr=subprocess.STDOUT,
        )

    if sys.stdin.readline():
        # Ignoring Ctrl+C only after starting evget keeps evget from inheriting it.
        win32api.SetConsoleCtrlHandler(None, True)
        win32api.GenerateConsoleCtrlEvent(win32con.CTRL_C_EVENT, 0)

    return process.wait()


if __name__ == "__main__":
    sys.exit(_console_helper(*sys.argv[1:]))
