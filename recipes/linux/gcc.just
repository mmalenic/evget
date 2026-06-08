# gcc recipes for evget, including the sanitizer suite (asan).
set working-directory := '../..'

import '../common.just'
import '../sanitizers.just'

profile_args := '-pr ./profiles/gcc'
backend_args := '-o evget/*:build_evgetx11=True -o evget/*:build_evgetlibinput=True'
compiler := 'gcc'
build_dir := 'build/gcc-' + lowercase(build_type)
