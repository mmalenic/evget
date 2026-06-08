# mingw recipes for evget, including the compatible sanitizer suite.
set windows-shell := ['cmd.exe', '/d', '/s', '/c']
set working-directory := '../..'

import '../common.just'
import '../sanitizers.just'

profile_args := '-pr:a profiles/gcc -c tools.cmake.cmaketoolchain:generator=Ninja'
backend_args := '-o evget/*:build_evgetwindows=True'
compiler := 'gcc'
build_dir := 'build/gcc-' + lowercase(build_type)
