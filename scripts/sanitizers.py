"""
Build and run the test suite using sanitizers. Running sanitizer tests
here will auto-detect the OS and run only those sanitizers applicable.
"""

from __future__ import annotations

import argparse
import json
import os
import shlex
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
OS = "windows" if sys.platform == "win32" else "linux"
EXE = "evgettest.exe" if OS == "windows" else "evgettest"

SANITIZER_ENV = {
    "UBSAN_OPTIONS": "print_stacktrace=1",
    "ASAN_OPTIONS": "check_initialization_order=1:strict_init_order=1",
}

def run(cmd, *, check=True, cwd=REPO, env=None, capture=False):
    """
    Run a subprocess from the repo root.
    """
    return subprocess.run(
        [str(c) for c in cmd],
        check=check,
        cwd=cwd,
        env=env,
        capture_output=capture,
        text=capture,
    )

@dataclass
class Ctx:
    """
    The build context per run.
    """

    compiler: str
    build_type: str
    profile_args: list[str]
    backend_args: list[str]
    gtest_filter: str
    opts: list[str]
    const: str | None = None

    def __post_init__(self) -> None:
        name = f"{self.compiler}-{self.build_type.lower()}"
        self.directory = (REPO / "build" / (f"{name}-{self.const}" if self.const is not None else name)).resolve()

    def conan_build(self, *, extra=(), env=None) -> Path:
        """
        Run `conan build` in this context's directory.
        """
        run(["conan", "profile", "detect", "-e"], env=env)
        layout = []

        if self.const is not None:
            folder = ["settings.compiler", "settings.build_type", f"const.{self.const}"]
            layout = ["-c", f"tools.cmake.cmake_layout:build_folder_vars={folder!r}"]

        run(
            [
                "conan", "build", ".", "--build=missing",
                "-s", f"build_type={self.build_type}",
                "-s", "compiler.cppstd=23",
                "-o", "evget/*:build_testing=True",
                *self.profile_args, *self.backend_args, *layout, *extra, *self.opts,
            ],
            env=env,
        )

        return self.directory


class Variant:
    """
    A sanitizer variant which builds and runs the tests.
    """

    name = ""
    oses: frozenset[str] = frozenset()
    env: dict[str, str] = {}
    # By default, run on both Debug and Release to catch as many issues as possible.
    build_types: tuple[str, ...] = ("Debug", "RelWithDebInfo")
    isolated = False

    def __init__(self, ctx: Ctx):
        self.ctx = ctx

    def build(self) -> Path:
        """
        Build the test.
        """
        raise NotImplementedError

    def check(self, exe: Path) -> int:
        """
        Run the test with the sanitizer.
        """
        return run(
            [exe, f"--gtest_filter={self.ctx.gtest_filter}"],
            cwd=exe.parent,
            check=False,
            env={**os.environ, **self.env} if self.env else None,
        ).returncode

    def do_check(self) -> int:
        """
        Build and run the test suite.
        """
        return self.check(self.build() / EXE)

class ClangAsan(Variant):
    """
    Clang asan variant.
    """

    name = "asan"
    oses = frozenset({"linux"})
    env = SANITIZER_ENV
    isolated = True

    FLAGS = ["-fsanitize=address,undefined,leak,integer", "-fno-sanitize-recover=all", "-fno-omit-frame-pointer"]

    def build(self) -> Path:
        return self.ctx.conan_build(extra=["-c", f"tools.build:cxxflags={self.FLAGS!r}"])


class ClangMsan(Variant):
    """
    Clang msan variant.
    """

    name = "msan"
    oses = frozenset({"linux"})
    env = SANITIZER_ENV
    isolated = True

    FLAGS = ["-fsanitize=memory", "-fno-omit-frame-pointer", "-fsanitize-memory-track-origins=2"]
    LLVM_REPO = "https://github.com/llvm/llvm-project.git"
    LLVM_CMAKE_ARGS = [
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DLLVM_ENABLE_RUNTIMES=libcxx;libcxxabi;libunwind",
        "-DLLVM_USE_SANITIZER=MemoryWithOrigins",
        "-DLIBCXX_INCLUDE_TESTS=OFF",
        "-DLIBCXXABI_INCLUDE_TESTS=OFF",
        "-DLIBUNWIND_INCLUDE_TESTS=OFF",
    ]

    def __init__(self, ctx: Ctx):
        super().__init__(ctx)

        self.build_llvm(self.ctx.directory)
        self.llvm = self.ctx.directory / "llvm-project" / "build"

        self.ignore = self.write_ignorelist(
            self.ctx.directory / "ignorelist.txt", "*/libunwind/*", f"{self.ctx.directory}/*"
        )

        self.home = self.ctx.directory / ".conan2"
        self.home.mkdir(parents=True, exist_ok=True)

        self.cxx_flags = [
            "-nostdinc++", "-stdlib=libc++", "-lc++abi",
            f"-L{self.llvm}/lib", f"-I{self.llvm}/include", f"-I{self.llvm}/include/c++/v1",
            f"-Wl,--rpath={self.llvm}/lib", f"-fsanitize-ignorelist={self.ignore}", *self.FLAGS,
        ]
        self.c_flags =  [f"-fsanitize-ignorelist={self.ignore}", *self.FLAGS]

    def build(self) -> Path:
        return self.ctx.conan_build(
            env={**os.environ, "CONAN_HOME": str(self.home)},
            extra=[
                "-c", f"tools.build:cxxflags={self.cxx_flags!r}",
                "-c", f"tools.build:cflags={self.c_flags!r}",
                "-c", f"tools.build:sharedlinkflags={self.FLAGS!r}",
                "-c", f"tools.build:exelinkflags={self.FLAGS!r}",
            ],
        )

    def build_llvm(self, directory: Path) -> None:
        """
        Build LLVM with msan instrumentation.
        """
        repo = directory / "llvm-project"

        if not repo.exists():
            show = run(["conan", "profile", "show", *self.ctx.profile_args, "-f", "json"], capture=True)
            major = json.loads(show.stdout)["host"]["settings"]["compiler.version"].split(".")[0]
            directory.mkdir(parents=True, exist_ok=True)
            run(["git", "clone", "--depth", "1", "--branch", f"release/{major}.x", self.LLVM_REPO, repo])

        build = repo / "build"
        if build.is_dir():
            return

        ignore = self.write_ignorelist(directory / "build-ignorelist.txt", "*/libunwind/*")

        run(
            [
                "cmake", "-G", "Ninja", "-S", repo / "runtimes", "-B", build,
                *self.LLVM_CMAKE_ARGS,
                f"-DCMAKE_C_FLAGS=-fsanitize-ignorelist={ignore}",
                f"-DCMAKE_CXX_FLAGS=-fsanitize-ignorelist={ignore}",
            ],
            env={**os.environ, "CC": "clang", "CXX": "clang++"},
        )
        run(["ninja", "-C", build, "cxx", "cxxabi"])

    @staticmethod
    def write_ignorelist(path: Path, *patterns: str) -> Path:
        """
        Write an ignorelist.
        """
        path.write_text("".join(f"src:{p}\n" for p in patterns))
        return path

class MsvcAsan(Variant):
    """
    MSVC asan build.
    """

    name = "asan"
    oses = frozenset({"windows"})
    isolated = True

    def build(self) -> Path:
        return self.ctx.conan_build(extra=["-o", "evget/*:enable_asan=True"])


class MsvcRuntimeChecks(Variant):
    """
    MSVC runtime checks.
    """

    name = "runtime-checks"
    oses = frozenset({"windows"})
    isolated = True
    # Only run on debug as it's a compiler requirement.
    build_types = ("Debug",)

    def build(self) -> Path:
        return self.ctx.conan_build(extra=["-o", "evget/*:msvc_runtime_checks=True"])


class PageHeap(Variant):
    """
    Run the tests under gflags full page heap.
    """

    name = "pageheap"
    oses = frozenset({"windows"})
    # Only run on release as it appends to an existing binary.
    build_types = ("RelWithDebInfo",)

    def build(self) -> Path:
        return self.ctx.conan_build()

    def check(self, exe: Path) -> int:
        run(["gflags", "/p", "/enable", EXE, "/full"])
        try:
            return super().check(exe)
        finally:
            run(["gflags", "/p", "/disable", EXE])


class AppVerifier(Variant):
    """
    Run the tests under app verifier.
    """

    name = "appverifier"
    oses = frozenset({"windows"})
    # Only run on release as it appends to an existing binary.
    build_types = ("RelWithDebInfo",)

    def build(self) -> Path:
        return self.ctx.conan_build()

    def check(self, exe: Path) -> int:
        run(["appverif", "-enable", "Heaps", "Handles", "Locks", "Memory", "-for", EXE])
        try:
            return super().check(exe)
        finally:
            run(["appverif", "-disable", "*", "-for", EXE])


def main() -> None:
    parser = argparse.ArgumentParser(description="Run the evget test suite under sanitizers.")
    parser.add_argument("action", choices=["run", "build", "clean", "list"])
    parser.add_argument("name", nargs="?", help="sanitizer name; omit to run or clean all")
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--build-type", default="Debug")
    parser.add_argument("--profile-args", default="")
    parser.add_argument("--backend-args", default="")
    parser.add_argument("--filter", default="*")
    parser.add_argument("--opts", default="")
    args = parser.parse_args()

    variants = [ClangAsan, ClangMsan, MsvcAsan, MsvcRuntimeChecks, PageHeap, AppVerifier]
    here = [cls for cls in variants if OS in cls.oses]

    if args.action == "list":
        print(" ".join(cls.name for cls in here))
        return

    def context(cls: type[Variant], build_type: str) -> Ctx:
        return Ctx(
            compiler=args.compiler,
            build_type=build_type,
            profile_args=shlex.split(args.profile_args),
            backend_args=shlex.split(args.backend_args),
            gtest_filter=args.filter,
            opts=shlex.split(args.opts),
            const=cls.name if cls.isolated else None,
        )

    def find(name: str) -> type[Variant]:
        cls = next((cls for cls in here if cls.name == name), None)
        if cls is None:
            parser.error(f"no `{name}` sanitizer on {OS}")
        return cls

    if args.action == "clean":
        if not args.name:
            for directory in {context(cls, build).directory for cls in here for build in cls.build_types}:
                shutil.rmtree(directory, ignore_errors=True)
            return
        shutil.rmtree(context(find(args.name), args.build_type).directory, ignore_errors=True)
        return

    if args.action == "run" and not args.name:
        matrix = [(cls, build) for cls in here for build in cls.build_types]
        for cls, build in matrix:
            rc = cls(context(cls, build)).do_check()
            if rc != 0:
                sys.exit(rc)
        return

    cls = find(args.name)
    v = cls(context(cls, args.build_type))
    if args.action == "build":
        v.build()
        return
    sys.exit(v.do_check())


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        sys.exit(e.returncode or 1)
