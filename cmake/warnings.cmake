# Warning flags for every evget, where target options apply only to evget code.

# Apply the evget warning to ``target`` with PRIVATE.
function(evget_apply_warnings target)
    set(gnu_warnings -Wall -Wextra -Wpedantic)
    set(msvc_warnings /W4 /permissive- /sdl)
    set(msvc_analyze /analyze /analyze:external- /external:env:INCLUDE /WX)

    # C6326 has false positives on gtest.
    if(target STREQUAL TEST_EXECUTABLE_NAME)
        list(APPEND msvc_analyze /wd6326)
    endif()
    # clang-cl needs /clang: prepended.
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        list(TRANSFORM gnu_warnings PREPEND "/clang:")
    endif()

    target_compile_options(
        ${target} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:${gnu_warnings}>
                          $<$<CXX_COMPILER_ID:MSVC>:${msvc_warnings}>
    )

    # Clang-tidy static analysis.
    if(EVGET_RUN_CLANG_TIDY)
        if(DEFINED EVGET_CLANG_TIDY_EXECUTABLE)
            set(clang_tidy "${EVGET_CLANG_TIDY_EXECUTABLE}")
        else()
            set(clang_tidy "clang-tidy")
        endif()
        if(EVGET_CLANG_TIDY_FIX_ERRORS)
            list(APPEND clang_tidy "--fix-errors")
        endif()
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${clang_tidy}")
    endif()

    # MSVC static analysis with /analyze, excluding external headers.
    if(EVGET_RUN_MSVC_ANALYZE)
        target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:${msvc_analyze}>)
    endif()
endfunction()
