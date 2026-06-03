# Shared recipes. Importing modules must define `profile_args` and `backend_args`.

# Build type, override with `just build_type=Release <recipe>`.
build_type := 'Debug'

# Detect the conan profile for building.
profile:
    conan profile detect -e

# A stale cache from another generator could otherwise shadow the toolchain we just configured.
# Clean CMakeCache.txt across flat and per-config build layouts.
clean_cache:
    cmake -E rm -f build/CMakeCache.txt build/Debug/CMakeCache.txt build/Release/CMakeCache.txt

# Build evget.
build *opts='': profile clean_cache
    conan build . --build=missing -s build_type={{ capitalize(build_type) }} -s compiler.cppstd=23 {{ profile_args }} {{ backend_args }} {{ opts }}

# Build only the evget library.
build_lib *opts='': (build '-o evget/*:build_bin=False ' + opts)

# Build evget with the test executable.
build_test *opts='': (build '-o evget/*:build_testing=True ' + opts)

# Rebuild evget using the existing CMake directory.
rebuild:
    cd build/{{ capitalize(build_type) }} && cmake --build .

# Build and run evget.
run args='--help' *opts='': (build opts)
    cd build/{{ capitalize(build_type) }} && {{ if os_family() == "windows" { "bin\\evget.exe" } else { "./bin/evget" } }} {{ args }}

# Build and test evget. cmd.exe needs `.\` and `.exe`; posix shells need `./`.
test filter='*' *opts='': (build_test opts)
    cd build/{{ capitalize(build_type) }} && {{ if os_family() == "windows" { ".\\evgettest.exe" } else { "./evgettest" } }} --gtest_filter="{{ filter }}"
