# Warning flags for every evget, where target options apply only to evget code.

# Apply the evget warning to ``target`` with PRIVATE.
function(evget_apply_warnings target)
    target_compile_options(
        ${target} PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
                          $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
    )

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
