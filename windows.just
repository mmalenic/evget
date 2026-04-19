# Windows module declaring compiler sub-modules

mod msvc 'windows/msvc.just'
mod clangcl 'windows/clang-cl.just'
mod mingw 'windows/mingw.just'

# Verify all three Windows toolchains build and test.
verify_all: msvc::build msvc::test clangcl::build clangcl::test mingw::build mingw::test
    pwsh -NoProfile -Command "Write-Host 'All Windows toolchains passed'"
