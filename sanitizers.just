# Test evlist using address and memory sanitizers. This needs to compile libcxx and libcxxabi with sanitiziers enabled.

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
build_llvm dir runtimes use_sanitizer targets $COMPILER_VERSION='20' *build_options='': (fetch_llvm COMPILER_VERSION)
    #!/usr/bin/env bash
    set -euxo pipefail

    cd llvm-project

    if [ -d "{{ dir }}" ]; then
        echo "llvm at {{ dir }} already built - skipping"
    else
        # Extract the compiler launcher from a string such as '-o "&:compiler_launcher=..."'
        launcher=$(echo {{ build_options }} | grep -Po 'compiler_launcher=\K(.*)(?=")?' | sed -e "s/\"\|'//g" || true)

        # Prevent libunwind from being sanitizer-instrumented.
        ignorelist="$(realpath ..)/build/Debug/ignorelist.txt"
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
_test_llvm dir symbolizer_path profile='./profiles/llvm_libc' filter='*' $COMPILER_VERSION='20' *build_options='':
    #!/usr/bin/env bash
    set -euxo pipefail

    # Each sanitizer variant needs its own conan cache as sanitizer-instrumented
    # dependencies cannot link to a different sanitizer. E.g. ASAN cannot link with
    # MSAN cache.
    export CONAN_HOME="$HOME/.conan2-{{ dir }}"
    mkdir -p "$CONAN_HOME"

    # Clear build/Debug to avoid issues when running asan/msan separately.
    rm -rf build/Debug
    dir=$(realpath llvm-project/{{ dir }})
    mkdir -p build/Debug
    # Keep this in sync with the ignorelist written in `build_llvm`.
    {
        echo 'src:*/libunwind/*'
        echo "src:$dir/*"
        echo "src:$CONAN_HOME/*"
    } > build/Debug/ignorelist.txt

    LLVM_DIR=$dir IGNORE_LIST=$(realpath build/Debug)/ignorelist.txt just build Debug {{ COMPILER_VERSION }} \
        -pr {{ profile }} '-o "&:build_testing=True" {{ build_options }}'

    cd build/Debug
    UBSAN_OPTIONS=print_stacktrace=1 ASAN_OPTIONS=check_initialization_order=1:strict_init_order=1 \
        {{ symbolizer_path }}=$(command -v llvm-symbolizer-{{ COMPILER_VERSION }}) \
        ./evgettest --gtest_filter={{ filter }}

# Run tests with valgrind.
test_valgrind filter='*' $COMPILER_VERSION='' *build_options='': \
    (build 'Debug' COMPILER_VERSION '-o "&:build_testing=True" ' + build_options)
    cd build/Debug && \
        valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
        --suppressions=../../profiles/valgrind.supp --error-exitcode=1 ./evgettest --gtest_filter={{ filter }}

# Run tests with valgrind for both Wayland and X11 evget components.
test_valgrind_linux filter='*' $COMPILER_VERSION='' *build_options='': \
    (test_valgrind filter COMPILER_VERSION '-o "&:build_evgetx11=True" -o "&:build_evgetlibinput=True" ' + build_options)

# Build llvm cxx and cxxabi with the memory sanitizer.
build_msan $COMPILER_VERSION='20' *build_options='': \
    (build_llvm "build-msan" "libcxx;libcxxabi;libunwind" "MemoryWithOrigins" "cxx cxxabi" \
    COMPILER_VERSION build_options)

# Run tests with the memory sanitizer.
test_msan filter='*' $COMPILER_VERSION='20' *build_options='': \
    (build_msan COMPILER_VERSION build_options) \
    (_test_llvm "build-msan" "MSAN_SYMBOLIZER_PATH" "./profiles/llvm_msan" filter COMPILER_VERSION build_options)

# Run tests with the memory sanitizer for both Wayland and X11 evget components.
test_msan_linux filter='*' $COMPILER_VERSION='20' *build_options='': \
    (test_msan filter COMPILER_VERSION '-o "&:build_evgetx11=True" -o "&:build_evgetlibinput=True" ' + build_options)

# Build llvm cxx and cxxabi with the address sanitizer.
build_asan $COMPILER_VERSION='20' *build_options='': \
    (build_llvm "build-asan" "libcxx;libcxxabi;libunwind" "Address;Undefined" "cxx cxxabi" \
    COMPILER_VERSION build_options)

# Run tests with the address sanitizer.
test_asan filter='*' $COMPILER_VERSION='20' *build_options='': \
    (build_asan COMPILER_VERSION build_options) \
    (_test_llvm "build-asan" "ASAN_SYMBOLIZER_PATH" "./profiles/llvm_asan" filter COMPILER_VERSION build_options)

# Run tests with the address sanitizer for both Wayland and X11 evget components.
test_asan_linux filter='*' $COMPILER_VERSION='20' *build_options='': \
    (test_asan filter COMPILER_VERSION '-o "&:build_evgetx11=True" -o "&:build_evgetlibinput=True" ' + build_options)

# Remove a built sanitizer directory (e.g. `just clean_sanitizer msan`).
clean_sanitizer name: clean_cache
    rm -rf llvm-project/build-{{ name }}
