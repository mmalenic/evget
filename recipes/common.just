# Shared recipes. Importing modules must define `profile_args`, `backend_args`, and `build_dir`, and set
# `windows-shell`, because just evaluates every module's variables on each run.

# Build type, override with `just build_type=Release <recipe>`.
build_type := 'Debug'

# Run Conan and the Python scripts from uv when it's on PATH, otherwise run Conan from the PATH directly.
# Override with EVGET_USE_UV=true|false, or `just use_uv=false <recipe>`.
use_uv := env('EVGET_USE_UV', trim(shell(if os_family() == 'windows' { 'where /q uv && echo true|| echo false' } else { 'command -v uv > /dev/null && echo true || echo false' })))
uv_run := if use_uv == 'true' { 'uv run --locked ' } else { '' }
pytest := if use_uv == 'true' { 'uv run --locked --group integration pytest' } else { 'python -m pytest' }

# Detect the conan profile for building.
profile:
    {{ uv_run }}conan profile detect -e

# Build evget.
build *opts='': profile
    {{ uv_run }}conan build . --build=missing -s build_type={{ capitalize(build_type) }} -s compiler.cppstd=23 {{ profile_args }} {{ backend_args }} {{ opts }}

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

# Build evget and run the integration tests.
integration *args='': build
    {{ pytest }} tests/integration --evget {{ build_dir }}/bin/evget{{ if os_family() == "windows" { ".exe" } else { "" } }} {{ args }}
