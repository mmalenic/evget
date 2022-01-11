set_local_parent(CURSOR_LIBRARY_NAME libcursor)
set_local_parent(CURSOR_TEST_NAME cursortest)

add_program(
    TARGET ${CURSOR_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    include
    SRCS
    include/Cursor/PositionX.h
    include/Cursor/PositionY.h
)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()