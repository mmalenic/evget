set(CURSOR_LIBRARY_NAME libcursor PARENT_SCOPE)
set(CURSOR_TEST_NAME cursortest PARENT_SCOPE)

add_program(
    TARGET ${CURSOR_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    include
    SRCS
    include/Cursor/PositionX.h
    include/Cursor/PositionY.h
)

add_subdirectory(src)
add_subdirectory(test)