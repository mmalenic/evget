# Linux module: builds the X11 and libinput backends with the default compiler.
set working-directory := '..'
# Windows isn't used here, but it's required to evaluate the variables with an error on windows.
set windows-shell := ['cmd.exe', '/d', '/s', '/c']

import 'common.just'
import 'check.just'

profile_args := '-pr:a default -pr:a profiles/base'
backend_args := '-o evget/*:build_evgetx11=True -o evget/*:build_evgetlibinput=True'
check_args := '-o evget/*:run_clang_tidy=True'
build_dir := 'build/' + shell('python scripts/detect_compiler.py') + '-' + lowercase(build_type)

mod gcc 'linux/gcc.just'
mod clang 'linux/clang.just'

# Run tests under valgrind against the X11 and libinput backends.
test_valgrind filter='*' *opts='': (build_test opts)
    cd {{ build_dir }} && \
        valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
        --suppressions=../../profiles/valgrind.supp --error-exitcode=1 ./evgettest --gtest_filter={{ filter }}
