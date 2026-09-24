set working-directory := '../..'
set allow-duplicate-recipes

import '../common.just'
import '../check.just'

profile_args := '-pr:h profiles/clang-cl -pr:b default -o boost/*:header_only=True'
backend_args := '-o evget/*:build_evgetwindows=True'
check_args := '-o evget/*:run_clang_tidy=True -o evget/*:clang_tidy_args=-checks=-readability-duplicate-include'
compiler := 'clang'
build_dir := 'build/windows-clang-' + lowercase(build_type)

test filter='*' *opts='': (build_test opts)
    cd {{ build_dir }} && xvfb-run --auto-servernum wine ./evgettest.exe --gtest_filter={{ quote(filter) }}
