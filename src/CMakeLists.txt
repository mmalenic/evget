add_program(
    TARGET ${LIBRARY_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    SRCS
    CommandLine/OptionFlag.cpp
    CommandLine/Parser.cpp
    Event/Field.cpp
    Event/Cursor/PositionX.cpp
    Event/Cursor/PositionY.cpp
    Event/Data.cpp
    Event/MouseMove.cpp
    ShutdownHandler.cpp
    UnsupportedOperationException.cpp
    InvalidCommandLineOption.cpp
)