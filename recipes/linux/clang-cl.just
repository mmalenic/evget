# clang-cl recipes for evget from a Linux host, cross-compiling to Windows.
set working-directory := '../..'
set allow-duplicate-recipes

import '../common.just'
import '../check.just'

profile_args := '-pr:h profiles/clang-cl -pr:b default'
backend_args := '-o evget/*:build_evgetwindows=True'
# -readability-duplicate-include is required as clang has a bug on duplicate in -fms-extensions mode.
check_args := '-o evget/*:run_clang_tidy=True -o evget/*:clang_tidy_args=-checks=-readability-duplicate-include'
compiler := 'clang'
build_dir := 'build/windows-clang-' + lowercase(build_type)

test filter='*' *opts='': (build_test opts)
    cd {{ build_dir }} && xvfb-run --auto-servernum wine ./evgettest.exe --gtest_filter={{ quote(filter) }}
