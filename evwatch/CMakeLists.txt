set_and_get_variable(SHUTDOWN_LIBRARY_NAME libshutdown)
set_and_get_variable(SHUTDOWN_TEST_EXECUTABLE_NAME libshutdowntest)

add_program(
    TARGET ${SHUTDOWN_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
        include/shutdown/EventWatcher.h
)
add_subdirectory(src)

#if (BUILD_TESTING)
#    add_subdirectory(test)
#endif()