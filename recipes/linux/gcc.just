# gcc recipes for evget.
set working-directory := '../..'

import '../common.just'

profile_args := '-pr ./profiles/gcc'
backend_args := '-o evget/*:build_evgetx11=True -o evget/*:build_evgetlibinput=True'
build_dir := 'build/gcc-' + lowercase(build_type)
