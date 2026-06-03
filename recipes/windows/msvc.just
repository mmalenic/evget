# msvc recipes for evget.
set windows-shell := ['cmd.exe', '/d', '/s', '/c']
set working-directory := '../..'

import '../common.just'
import '../check.just'

profile_args := '-pr:a profiles/msvc -c tools.cmake.cmaketoolchain:generator=Ninja'
backend_args := '-o evget/*:build_evgetwindows=True'
check_args := '-o evget/*:run_msvc_analyze=True'
build_dir := 'build/msvc-' + lowercase(build_type)
