# mingw recipes for evget.
set shell := ['pwsh', '-NoProfile', '-Command']

# Build evget with mingw.
build build_type='Debug' *opts='':
    $env:COMPILER_VERSION = '14'
    conan install . -pr:a profiles/mingw -s build_type={{ build_type }} -s compiler.cppstd=23 \
        --build=missing -o "&:build_evgetwindows=True" {{ opts }}
    cmake --preset conan-{{ lowercase(build_type) }} -G Ninja
    cmake --build --preset conan-{{ lowercase(build_type) }}

# Test evget with mingw.
test filter='*' *opts='':
    $env:COMPILER_VERSION = '14'
    conan install . -pr:a profiles/mingw -s build_type=Debug -s compiler.cppstd=23 \
        --build=missing -o "&:build_evgetwindows=True" -o "&:build_testing=True" {{ opts }}
    cmake --preset conan-debug -G Ninja
    cmake --build --preset conan-debug
    .\build\Debug\evgettest.exe --gtest_filter={{ filter }}

# Check evget with mingw.
check *opts='':
    Write-Host "todo"

clean:
    if (Test-Path build) { Remove-Item -Recurse -Force build }
