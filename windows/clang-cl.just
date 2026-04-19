# clang-cl recipes for evget.
set shell := ['pwsh', '-NoProfile', '-Command']

# Build evget with clang-cl.
build build_type='Debug' *opts='':
    $env:COMPILER_VERSION = '19'
    conan install . -pr:a profiles/clang-cl -s build_type={{ build_type }} -s compiler.cppstd=23 \
        --build=missing -o "&:build_evgetwindows=True" {{ opts }}
    cmake --preset conan-{{ lowercase(build_type) }}
    cmake --build --preset conan-{{ lowercase(build_type) }}

# Test evget with clang-cl.
test filter='*' *opts='':
    $env:COMPILER_VERSION = '19'
    conan install . -pr:a profiles/clang-cl -s build_type=Debug -s compiler.cppstd=23 \
        --build=missing -o "&:build_evgetwindows=True" -o "&:build_testing=True" {{ opts }}
    cmake --preset conan-debug
    cmake --build --preset conan-debug
    .\build\Debug\evgettest.exe --gtest_filter={{ filter }}

# Check evget with clang-cl.
check *opts='':
    $env:COMPILER_VERSION = '19'
    conan install . -pr:a profiles/clang-cl -s build_type=Debug -s compiler.cppstd=23 \
        --build=missing -o "&:build_evgetwindows=True" -o "&:build_testing=True" \
        -o "&:run_clang_tidy=True" {{ opts }}
    cmake --preset conan-debug
    cmake --build --preset conan-debug

clean:
    if (Test-Path build) { Remove-Item -Recurse -Force build }
