"""
Print the Conan-detected default compiler for naming per-compiler build directories
"""

import json
import subprocess


def main() -> None:
    show = ["conan", "profile", "show", "-f", "json"]
    try:
        out = subprocess.run(show, capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError:
        subprocess.run(
            ["conan", "profile", "detect", "-e"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        out = subprocess.run(show, capture_output=True, text=True, check=True)

    print(json.loads(out.stdout)["host"]["settings"]["compiler"], end="")


if __name__ == "__main__":
    main()
