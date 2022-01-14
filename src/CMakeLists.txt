add_program(
    TARGET ${LIBRARY_NAME}
    SRCS
    CommandLine/OptionFlag.cpp
    CommandLine/Parser.cpp
    Event/Button/Press.cpp
    Event/Common/Time.cpp
    Event/Cursor/PositionX.cpp
    Event/Cursor/PositionY.cpp
    Event/Data.cpp
    Event/Field.cpp
    ShutdownHandler.cpp
    UnsupportedOperationException.cpp
    CommandLine/InvalidCommandLineOption.cpp
)