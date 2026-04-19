# Apply Windows macros to set compatibility behaviour.
function(evget_apply_windows_defines target)
    target_compile_definitions(
        ${target}
        PRIVATE UNICODE
                _UNICODE
                NOMINMAX
                WIN32_LEAN_AND_MEAN
                _WIN32_WINNT=0x0A00
                WINVER=0x0A00
                BOOST_ALL_NO_LIB
    )
endfunction()
