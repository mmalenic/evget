set(EVENT_LIBRARY_NAME libevent PARENT_SCOPE)
set(EVENT_TEST_NAME eventtest PARENT_SCOPE)

add_program(
    TARGET ${EVENT_LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    include
    SRCS
    include/Event/Data.h
    include/Event/Field.h
    include/Event/MouseMove.h
)

add_subdirectory(src)
add_subdirectory(test)

add_subdirectory(Cursor)

program_dependencies(TARGET ${EVENT_LIBRARY_NAME} DEP PUBLIC ${CURSOR_LIBRARY_NAME} ONLY_LINK)

set(CURSOR_LIBRARY_NAME ${CURSOR_LIBRARY_NAME} PARENT_SCOPE)
set(CURSOR_TEST_NAME ${CURSOR_TEST_NAME} PARENT_SCOPE)