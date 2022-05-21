set_and_get_variable(EVWATCH_LIBRARY_NAME libevwatch)
set_and_get_variable(EVWATCH_TEST_EXECUTABLE_NAME libevwatchtest)

add_program(
    TARGET ${EVWATCH_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/evwatch/EventWatcher.h
    include/evwatch/EventWatcherException.h
)
add_subdirectory(src)

#if (BUILD_TESTING)
#    add_subdirectory(test)
#endif()