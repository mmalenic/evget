# clang-cl recipes for evget, including the compatible sanitizer suite.
set windows-shell := ['cmd.exe', '/d', '/s', '/c']
set working-directory := '../..'

import '../common.just'
import '../check.just'
import '../sanitizers.just'

profile_args := '-pr:h profiles/clang-cl -pr:b profiles/msvc -c tools.cmake.cmaketoolchain:generator=Ninja'
backend_args := '-o evget/*:build_evgetwindows=True'
check_args := '-o evget/*:run_clang_tidy=True'
compiler := 'clang'
build_dir := 'build/clang-' + lowercase(build_type)
