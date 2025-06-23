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
        launcher=$(echo {{ build_options }} | grep -Po 'compiler_launcher=\K(.*)(?=")?' | sed -e "s/\"\|'//g")

        CC=clang-{{ COMPILER_VERSION }} CXX=clang++-{{ COMPILER_VERSION }} \
            cmake -G Ninja -S runtimes -B {{ dir }} \
            -DCMAKE_BUILD_TYPE=Debug \
            -DLLVM_ENABLE_RUNTIMES="{{ runtimes }}" \
            -DLLVM_USE_SANITIZER="{{ use_sanitizer }}" \
            -DCMAKE_C_COMPILER_LAUNCHER="$launcher" \
            -DCMAKE_CXX_COMPILER_LAUNCHER="$launcher"
        ninja -C {{ dir }} {{ targets }}
    fi

# Create the ignorelist for sanitizer tests to ignore non-project code.
create_ignore_list dir:
    mkdir -p build/Debug && echo "src:{{ dir }}/*" >> build/Debug/ignorelist.txt

# Error if the log file from sanitizers is non-empty.
error_non_empty_log:
    #!/usr/bin/env bash
    set -euxo pipefail

    if [ -f log ] && [ -s log ]; then
        cat log
        exit 1
    fi

# Test using an llvm sanitizer build directory.
_test_llvm dir symbolizer_path profile='./profiles/llvm_libc' filter='*' $COMPILER_VERSION='20' *build_options='':
    #!/usr/bin/env bash
    set -euxo pipefail

    dir=$(realpath llvm-project/{{ dir }})
    just create_ignore_list "$dir"

    LLVM_DIR=$dir IGNORE_LIST=$(realpath build/Debug)/ignorelist.txt just build Debug {{ COMPILER_VERSION }} \
        -pr {{ profile }} '-o "&:build_testing=True" {{ build_options }}'

    cd build/Debug
    {{ symbolizer_path }}=llvm-symbolizer-{{ COMPILER_VERSION }} ./evgettest --gtest_filter={{ filter }} 2> log
    just error_non_empty_log

# Run tests with the memory sanitizer.
test_valgrind filter='*' $COMPILER_VERSION='' *build_options='': \
    (build 'Debug' COMPILER_VERSION '-o "&:build_testing=True" ' + build_options)
    cd build/Debug && \
        valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=1  \
        ./evgettest --gtest_filter={{ filter }}

# Build llvm cxx and cxxabi with the memory sanitizer.
build_msan $COMPILER_VERSION='20' *build_options='': \
    (build_llvm "build-msan" "libcxx;libcxxabi;libunwind" "MemoryWithOrigins" "cxx cxxabi" \
    COMPILER_VERSION build_options)

# Run tests with the memory sanitizer.
test_msan filter='*' $COMPILER_VERSION='20' *build_options='': \
    (build_msan COMPILER_VERSION build_options) \
    (_test_llvm "build-msan" "MSAN_SYMBOLIZER_PATH" "./profiles/llvm_msan" filter COMPILER_VERSION build_options)

# Remove the built msan directory.
clean_msan: clean_cache
    rm -rf llvm-project/build-msan

# Build llvm cxx and cxxabi with the address sanitizer.
build_asan $COMPILER_VERSION='20' *build_options='': \
    (build_llvm "build-asan" "libcxx;libcxxabi;libunwind" "Address;Undefined" "cxx cxxabi" \
    COMPILER_VERSION build_options)

# Run tests with the address sanitizer.
test_asan filter='*' $COMPILER_VERSION='20' *build_options='': \
    (build_asan COMPILER_VERSION build_options) \
    (_test_llvm "build-asan" "ASAN_SYMBOLIZER_PATH" "./profiles/llvm_asan" filter COMPILER_VERSION build_options)

# Remove the built asan directory.
clean_asan: clean_cache
    rm -rf llvm-project/build-asan
