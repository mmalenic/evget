# msvc recipes for evget.
set shell := ['pwsh', '-NoProfile', '-Command']

# Build evget with msvc.
build build_type='Debug' *opts='':
    conan install . -pr:a profiles/msvc -s build_type={{ build_type }} -s compiler.cppstd=23 \
        --build=missing -o "&:build_evgetwindows=True" {{ opts }}
    cmake --preset conan-{{ lowercase(build_type) }}
    cmake --build --preset conan-{{ lowercase(build_type) }}

# Build evget with msvc.
test filter='*' *opts='':
    conan install . -pr:a profiles/msvc -s build_type=Debug -s compiler.cppstd=23 \
        --build=missing -o "&:build_evgetwindows=True" -o "&:build_testing=True" {{ opts }}
    cmake --preset conan-debug
    cmake --build --preset conan-debug
    .\build\Debug\evgettest.exe --gtest_filter={{ filter }}

# Build evget with msvc.
check *opts='':
    Write-Host "todo"

# Remove msvc builds.
clean:
    if (Test-Path build) { Remove-Item -Recurse -Force build }
