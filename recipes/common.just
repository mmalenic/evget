# Shared recipes. Importing modules must define `profile_args`, `backend_args`, and `build_dir`.

# Build type, override with `just build_type=Release <recipe>`.
build_type := 'Debug'

# Detect the conan profile for building.
profile:
    conan profile detect -e

# Build evget.
build *opts='': profile
    conan build . --build=missing -s build_type={{ capitalize(build_type) }} -s compiler.cppstd=23 {{ profile_args }} {{ backend_args }} {{ opts }}

# Build only the evget library.
build_lib *opts='': (build '-o evget/*:build_bin=False ' + opts)

# Build evget with the test executable.
build_test *opts='': (build '-o evget/*:build_testing=True ' + opts)

# Rebuild evget using the existing CMake directory.
rebuild:
    cd {{ build_dir }} && cmake --build .

# Build and run evget.
run args='--help' *opts='': (build opts)
    cd {{ build_dir }} && {{ if os_family() == "windows" { "bin\\evget.exe" } else { "./bin/evget" } }} {{ args }}

# Build and test evget. cmd needs `.\` and `.exe` and posix needs `./`.
test filter='*' *opts='': (build_test opts)
    cd {{ build_dir }} && {{ if os_family() == "windows" { ".\\evgettest.exe --gtest_filter=" + filter } else { "./evgettest --gtest_filter=" + quote(filter) } }}
