# clang recipes for evget, including the libc++ sanitizer flow (needs clang and a sanitizer-instrumented libc++).
set working-directory := '../..'

import '../common.just'
import '../check.just'

profile_args := '-pr ./profiles/clang'
backend_args := '-o evget/*:build_evgetx11=True -o evget/*:build_evgetlibinput=True'
check_args := '-o evget/*:run_clang_tidy=True'
build_dir := 'build/clang-' + lowercase(build_type)

# Fetch the llvm repo for building sanitized binaries.
fetch_llvm $CLANG_VERSION='20':
    #!/usr/bin/env bash
    set -euxo pipefail

    if [ -d "llvm-project" ]; then
        echo "llvm already fetched - skipping"
    else
        version=$(clang-{{ CLANG_VERSION }} --version | grep -Po '(\d+).(\d+).(\d+)' | head -1)
        git clone --depth 1 --branch llvmorg-"$version" https://github.com/llvm/llvm-project.git
    fi

# Build llvm with options.
build_llvm dir runtimes use_sanitizer targets $COMPILER_VERSION='20' *opts='': (fetch_llvm COMPILER_VERSION)
    #!/usr/bin/env bash
    set -euxo pipefail

    cd llvm-project

    if [ -d "{{ dir }}" ]; then
        echo "llvm at {{ dir }} already built - skipping"
    else
        # Extract the compiler launcher from a string such as '-o evget/*:compiler_launcher=...'
        launcher=$(echo {{ opts }} | grep -Po 'compiler_launcher=\K(.*)(?=")?' | sed -e "s/\"\|'//g" || true)

        # Prevent libunwind from being sanitizer-instrumented.
        ignorelist="$(realpath ..)/{{ build_dir }}/ignorelist.txt"
        mkdir -p "$(dirname "$ignorelist")"
        echo 'src:*/libunwind/*' > "$ignorelist"

        CC=clang-{{ COMPILER_VERSION }} CXX=clang++-{{ COMPILER_VERSION }} \
            cmake -G Ninja -S runtimes -B {{ dir }} \
            -DCMAKE_BUILD_TYPE=Debug \
            -DLLVM_ENABLE_RUNTIMES="{{ runtimes }}" \
            -DLLVM_USE_SANITIZER="{{ use_sanitizer }}" \
            -DLIBCXX_INCLUDE_TESTS=OFF \
            -DLIBCXXABI_INCLUDE_TESTS=OFF \
            -DLIBUNWIND_INCLUDE_TESTS=OFF \
            -DCMAKE_C_FLAGS="-fsanitize-ignorelist=$ignorelist" \
            -DCMAKE_CXX_FLAGS="-fsanitize-ignorelist=$ignorelist" \
            -DCMAKE_C_COMPILER_LAUNCHER="$launcher" \
            -DCMAKE_CXX_COMPILER_LAUNCHER="$launcher"
        ninja -C {{ dir }} {{ targets }}
    fi

# Test using an llvm sanitizer build directory.
_test_llvm dir symbolizer_path profile='./profiles/llvm_libc' filter='*' $COMPILER_VERSION='20' *opts='':
    #!/usr/bin/env bash
    set -euxo pipefail

    # Each sanitizer variant needs its own conan cache as sanitizer-instrumented
    # dependencies cannot link to a different sanitizer. E.g. ASAN cannot link with
    # MSAN cache.
    export CONAN_HOME="$HOME/.conan2-{{ dir }}"
    mkdir -p "$CONAN_HOME"

    # Clear the build dir to avoid issues when running asan/msan separately.
    rm -rf {{ build_dir }}
    dir=$(realpath llvm-project/{{ dir }})
    mkdir -p {{ build_dir }}
    # Keep this in sync with the ignorelist written in `build_llvm`.
    {
        echo 'src:*/libunwind/*'
        echo "src:$dir/*"
        echo "src:$CONAN_HOME/*"
    } > {{ build_dir }}/ignorelist.txt

    LLVM_DIR=$dir IGNORE_LIST=$(realpath {{ build_dir }})/ignorelist.txt just build_test \
        -pr {{ profile }} {{ opts }}

    cd {{ build_dir }}
    UBSAN_OPTIONS=print_stacktrace=1 ASAN_OPTIONS=check_initialization_order=1:strict_init_order=1 \
        {{ symbolizer_path }}=$(command -v llvm-symbolizer-{{ COMPILER_VERSION }}) \
        ./evgettest --gtest_filter={{ filter }}

# Build llvm cxx and cxxabi with the memory sanitizer.
build_msan $COMPILER_VERSION='20' *opts='': \
    (build_llvm "build-msan" "libcxx;libcxxabi;libunwind" "MemoryWithOrigins" "cxx cxxabi" \
    COMPILER_VERSION opts)

# Run tests with the memory sanitizer against the X11 and libinput backends.
test_msan filter='*' $COMPILER_VERSION='20' *opts='': \
    (build_msan COMPILER_VERSION opts) \
    (_test_llvm "build-msan" "MSAN_SYMBOLIZER_PATH" "./profiles/llvm_msan" filter COMPILER_VERSION \
    '-o evget/*:build_evgetx11=True -o evget/*:build_evgetlibinput=True ' + opts)

# Build llvm cxx and cxxabi with the address sanitizer.
build_asan $COMPILER_VERSION='20' *opts='': \
    (build_llvm "build-asan" "libcxx;libcxxabi;libunwind" "Address;Undefined" "cxx cxxabi" \
    COMPILER_VERSION opts)

# Run tests with the address sanitizer against the X11 and libinput backends.
test_asan filter='*' $COMPILER_VERSION='20' *opts='': \
    (build_asan COMPILER_VERSION opts) \
    (_test_llvm "build-asan" "ASAN_SYMBOLIZER_PATH" "./profiles/llvm_asan" filter COMPILER_VERSION \
    '-o evget/*:build_evgetx11=True -o evget/*:build_evgetlibinput=True ' + opts)

# Remove a built sanitizer directory (e.g. `just linux::clang::clean_sanitizer msan`).
clean_sanitizer name:
    rm -rf {{ build_dir }} llvm-project/build-{{ name }}
