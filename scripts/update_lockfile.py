"""
Update the lockfile, allowing cross-platform dependencies to take effect from any platform.
"""

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


def main() -> None:
    # Remove the existing lock so that it's re-generated completely.
    Path("conan.lock").unlink(missing_ok=True)

    locks = []
    for profile, arch, backend in PLATFORMS:
        lock = f"lock.{profile}-{arch}-{backend}.lock"
        locks.append(lock)
        subprocess.run(
            [
                "conan",
                "lock",
                "create",
                ".",
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
        )

    merge = ["conan", "lock", "merge", "--lockfile-out", "conan.lock"]
    for lock in locks:
        merge.extend(["--lockfile", lock])
    subprocess.run(merge, check=True)

    for lock in locks:
        Path(lock).unlink(missing_ok=True)


if __name__ == "__main__":
    main()
