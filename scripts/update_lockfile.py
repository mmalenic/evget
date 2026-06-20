"""
Update the lockfile, allowing cross-platform dependencies to take effect from any platform.
"""

import os
import subprocess
from pathlib import Path

# Probably slightly overkill but different architectures and platforms have
# different dependencies sometimes.
PLATFORMS = [
    ("clang", "x86_64", "libinput"),
    ("clang", "x86_64", "x11"),
    ("clang", "armv8", "libinput"),
    ("clang", "armv8", "x11"),
    ("gcc", "x86_64", "libinput"),
    ("gcc", "x86_64", "x11"),
    ("gcc", "armv8", "libinput"),
    ("gcc", "armv8", "x11"),
    ("msvc", "x86_64", "windows"),
    ("msvc", "armv8", "windows"),
    ("clang-cl", "x86_64", "windows"),
    ("clang-cl", "armv8", "windows"),
    ("gcc", "x86_64", "windows"),
]

# The locks need the os and arch to be present to compute for that platform.
PROFILE_SETTINGS = {
    "clang": ("clang", "19"),
    "clang-cl": ("clang", "19"),
    "gcc": ("gcc", "14"),
    "msvc": ("msvc", "194"),
}


def main() -> None:
    locks = []
    try:
        for profile, arch, backend in PLATFORMS:
            compiler, version = PROFILE_SETTINGS[profile]
            lock = f"lock.{profile}-{arch}-{backend}.lock"
            locks.append(lock)

            # Use environment only to compute lock files so that they are self-contained.
            env = {
                **os.environ,
                "EVGET_OS": "Windows" if backend == "windows" else "Linux",
                "EVGET_ARCH": arch,
                "EVGET_COMPILER": compiler,
                "EVGET_VERSION": version,
            }

            subprocess.run(
                [
                    "conan",
                    "lock",
                    "create",
                    ".",
                    "--lockfile",
                    "",
                    "--lockfile-out",
                    lock,
                    "-pr:a",
                    f"profiles/{profile}",
                    "-s:a",
                    f"arch={arch}",
                    "-o",
                    f"&:build_evget{backend}=True",
                ],
                check=True,
                env=env,
            )

        merge = ["conan", "lock", "merge", "--lockfile-out", "conan.lock.tmp"]
        for lock in locks:
            merge.extend(["--lockfile", lock])
        subprocess.run(merge, check=True)

        # Replace the lock only once complete.
        os.replace("conan.lock.tmp", "conan.lock")
    finally:
        for lock in locks:
            Path(lock).unlink(missing_ok=True)
        Path("conan.lock.tmp").unlink(missing_ok=True)


if __name__ == "__main__":
    main()
