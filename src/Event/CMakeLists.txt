add_program(
    TARGET ${LIBRARY_NAME}
    SRCS
    Field.cpp
    Cursor/PositionX.cpp
    Cursor/PositionY.cpp
    Data.cpp
    MouseMove.cpp
)

add_subdirectory(Button)
add_subdirectory(Cursor)