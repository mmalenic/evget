# Warning flags for every evget, where target options apply only to evget code.

# Apply the evget warning to ``target`` with PRIVATE.
function(evget_apply_warnings target)
    target_compile_options(
        ${target} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
                          $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
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

    # MSVC static analysis with /analyze.
    if(EVGET_RUN_MSVC_ANALYZE)
        target_compile_options(${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/analyze;/WX>)
    endif()

    # The MSVC runtime checks.
    if(EVGET_MSVC_RUNTIME_CHECKS)
        target_compile_options(
            ${target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/GS;/sdl>
                              $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/RTC1>
        )
        target_compile_definitions(
            ${target} PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:EVGET_CRT_DEBUG_HEAP=1>
        )
    endif()
endfunction()
