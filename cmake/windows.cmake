# Apply Windows macros to set compatibility behaviour.
function(evget_apply_windows_defines target)
    # Boost.Asio and Boost.WinAPI choose code paths from these, so every TU that includes libevget should have these.
    target_compile_definitions(
        ${target} PUBLIC _WIN32_WINNT=0x0A00 WINVER=0x0A00 BOOST_USE_WINDOWS_H NOMINMAX WIN32_LEAN_AND_MEAN
    )
    target_compile_definitions(${target} PRIVATE UNICODE _UNICODE BOOST_ALL_NO_LIB)
    # This keeps the IDE compilation database happier as without __cplusplus stays incorrectly as 199711L.
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        target_compile_options(${target} PUBLIC /Zc:__cplusplus)
    endif()
endfunction()

# On windows DLLs must sit next to the executable for the loader to find them, so copy them here.
function(evget_copy_runtime_dlls target)
    if(NOT WIN32)
        return()
    endif()

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${target}> $<TARGET_RUNTIME_DLLS:${target}>
        COMMAND_EXPAND_LISTS
        COMMENT "copying runtime dlls next to ${target}"
    )
endfunction()
