# Windows module: builds the Windows backend with the default compiler (msvc).
set windows-shell := ['cmd.exe', '/d', '/s', '/c']
set working-directory := '..'

import 'common.just'
import 'check.just'

profile_args := '-pr:a default -pr:a profiles/base -c tools.cmake.cmaketoolchain:generator=Ninja'
backend_args := '-o evget/*:build_evgetwindows=True'
check_args := '-o evget/*:run_msvc_analyze=True'
compiler := shell('python scripts/detect_compiler.py')
build_dir := 'build/' + compiler + '-' + lowercase(build_type)

mod msvc 'windows/msvc.just'
mod clangcl 'windows/clang-cl.just'
mod mingw 'windows/mingw.just'
