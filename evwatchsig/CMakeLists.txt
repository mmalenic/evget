set_and_get_variable(SHUTDOWN_SIGNAL_LIBRARY_NAME libshutdownsignal)
set_and_get_variable(SHUTDOWN_SIGNAL_TEST_EXECUTABLE_NAME libshutdownsignaltest)

add_program(
    TARGET ${SHUTDOWN_SIGNAL_LIBRARY_NAME} MAKE_LIBRARY
    FEATURE_REQUIRES signal.h
    FEATURE_CHECK SHUTDOWN_SIGNAL_HAS_DEPENDENCIES
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
        include/shutdownsignal/EventWatcherSignal.h
)
add_subdirectory(src)

add_library_and_link(SHUTDOWN_SIGNAL_LIBRARY_NAME SHUTDOWN_LIBRARY_NAME "")

if (BUILD_TESTING)
    add_subdirectory(test)
endif()