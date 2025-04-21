import 'sanitizers.just'

# Detect the conan profile for building.
profile:
    conan profile detect -e

# Update the lock file.
update:
    conan lock create . --lockfile-clean

# Build evget.
build build_type='Debug' $COMPILER_VERSION='' *build_options='': profile clean_cache
    conan build . --build=missing -s build_type={{ capitalize(build_type) }} -s compiler.cppstd=23 {{ build_options }}

# Build with the clang profile.
build_clang build_type='Debug' $COMPILER_VERSION='19' *build_options='': \
    (build build_type COMPILER_VERSION '-pr ./profiles/clang ' + build_options)

# Build with the gcc profile.
build_gcc build_type='Debug' $COMPILER_VERSION='14' *build_options='': \
    (build build_type COMPILER_VERSION '-pr ./profiles/gcc ' + build_options)

# Rebuild evget using the existing CMake directory.
rebuild build_type='Debug':
    cd build/{{ capitalize(build_type) }} && cmake --build .

# Build only the evget library.
build_lib build_type='Debug' $COMPILER_VERSION='' *build_options='': \
    (build build_type COMPILER_VERSION '-o "&:build_bin=False" ' + build_options)

# Build and install evget.
install build_type='Release' $COMPILER_VERSION='' *build_options='': \
    (build build_type COMPILER_VERSION build_options)
    cmake --install build/{{ capitalize(build_type) }}

# Build and package the binary in a compressed file.
package build_type='Release' name='evget.tar.gz' $COMPILER_VERSION='' *build_options='': \
    (build build_type COMPILER_VERSION build_options)
    cd build/{{ capitalize(build_type) }} && tar -czf {{ name }} evget && sha256sum -b {{ name }} > {{ name }}.sha256

# Build and run evget.
run args='--help' build_type='Release' $COMPILER_VERSION='' *build_options='': \
    (build build_type COMPILER_VERSION build_options)
    cd build/{{ capitalize(build_type) }} && ./evget {{ args }}

_run_tests filter='*':
    cd build/Debug && ./evgettest --gtest_filter={{ filter }}

# Build and test evget.
test filter='*' $COMPILER_VERSION='' *build_options='': \
    (build 'Debug' COMPILER_VERSION '-o "&:build_testing=True" ' + build_options) (_run_tests filter)

# Build and test evget using the clang profile.
test_clang filter='*' $COMPILER_VERSION='19' *build_options='': \
    (build_clang 'Debug' COMPILER_VERSION '-o "&:build_testing=True" ' + build_options) (_run_tests filter)

# Build and test evget using the gcc profile.
test_gcc filter='*' $COMPILER_VERSION='14' *build_options='': \
    (build_gcc 'Debug' COMPILER_VERSION '-o "&:build_testing=True" ' + build_options) (_run_tests filter)

# Run pre-commit and other lints.
lint:
    pre-commit run --all-files

# Run clang tidy on code.
check $COMPILER_VERSION='' *build_options='': \
    lint (build 'Debug' COMPILER_VERSION '-o "&:build_testing=True" -o "&:run_clang_tidy=True" ' + build_options)

# Remove the build directory.
clean:
    rm -rf build

# Clean the CMakeCache.txt only.
clean_cache:
    rm -f build/Debug/CMakeCache.txt && rm -f build/Release/CMakeCache.txt

# Generate documentation.
doc:
    git clone --depth 1 --branch v2.3.4 https://github.com/jothepro/doxygen-awesome-css.git build/doxygen-awesome-css || true
    doxygen
