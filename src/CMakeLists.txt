add_program(
    TARGET ${LIBRARY_NAME}
    SRCS
    CommandLine/OptionFlag.cpp
    CommandLine/Parser.cpp
    Event/Cursor/PositionX.cpp
    Event/Cursor/PositionY.cpp
    Event/Data.cpp
    Event/Field.cpp
    Event/MouseMove.cpp
    ShutdownHandler.cpp
    UnsupportedOperationException.cpp
    InvalidCommandLineOption.cpp
)