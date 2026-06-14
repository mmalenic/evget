# Warning flags for every evget, where target options apply only to evget code.

# Apply the evget warning to ``target`` with PRIVATE.
function(evget_apply_warnings target)
    set(gnu_warnings -Wall -Wextra -Wpedantic)
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        # clang-cl needs /clang: prepended
        list(TRANSFORM gnu_warnings PREPEND "/clang:")
    endif()

    target_compile_options(
        ${target} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:${gnu_warnings}>
                          $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive-;/sdl>
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

    # MSVC static analysis with /analyze. Exclude SDK/external headers and silence C6326 from
    # GoogleTest macro expansions, which are attributed to project lines.
    if(EVGET_RUN_MSVC_ANALYZE)
        target_compile_options(
            ${target}
            PRIVATE
                $<$<CXX_COMPILER_ID:MSVC>:/analyze;/analyze:external-;/external:env:INCLUDE;/wd6326;/WX>
        )
    endif()
endfunction()
