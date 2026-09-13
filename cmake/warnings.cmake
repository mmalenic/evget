# Warning flags for every evget, where target options apply only to evget code.

# Apply the evget warning to ``target`` with PRIVATE.
function(evget_apply_warnings target)
    set(warnings_gnu -Wall -Wextra -Wpedantic)
    set(warnings_msvc /W4 /permissive- /sdl)
    set(analyze_msvc /analyze /analyze:external- /external:env:INCLUDE /WX)

    # C6326 has false positives on gtest.
    if(target STREQUAL TEST_EXECUTABLE_NAME)
        list(APPEND analyze_msvc /wd6326)
    endif()
    # clang-cl needs /clang: prepended.
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        list(TRANSFORM warnings_gnu PREPEND "/clang:")
    endif()

    target_compile_options(
        ${target} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:${warnings_gnu}>
                          $<$<CXX_COMPILER_ID:MSVC>:${warnings_msvc}>
    )

    # MSVC static analysis with /analyze, excluding external headers.
    if(EVGET_RUN_MSVC_ANALYZE)
        target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:${analyze_msvc}>)
    endif()
endfunction()
