# clang recipes for evget, including the sanitizer suite (asan, msan).
set working-directory := '../..'

import '../common.just'
import '../check.just'
import '../sanitizers.just'

profile_args := '-pr ./profiles/clang'
backend_args := '-o evget/*:build_evgetx11=True -o evget/*:build_evgetlibinput=True'
check_args := '-o evget/*:run_clang_tidy=True'
compiler := 'clang'
build_dir := 'build/clang-' + lowercase(build_type)
