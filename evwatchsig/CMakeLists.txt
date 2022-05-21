set_and_get_variable(EVWATCHSIG_LIBRARY_NAME libevwatchsig)
set_and_get_variable(EVWATCHSIG_TEST_EXECUTABLE_NAME libevwatchsigtest)

add_program(
    TARGET ${EVWATCHSIG_LIBRARY_NAME} MAKE_LIBRARY
    FEATURE_REQUIRES csignal
    FEATURE_CHECK EVWATCHSIG_HAS_DEPENDENCIES
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/evwatchsig/EventWatcherSignal.h
)
add_subdirectory(src)

add_library_and_link(EVWATCHSIG_LIBRARY_NAME EVWATCH_LIBRARY_NAME "")

if (BUILD_TESTING)
    add_subdirectory(test)
endif()