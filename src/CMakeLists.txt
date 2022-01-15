add_program(
    TARGET ${LIBRARY_NAME}
    SRCS
    CommandLine/OptionFlag.cpp
    CommandLine/Parser.cpp
    Event/Button/Character.cpp
    Event/Button/Press.cpp
    Event/Button/Release.cpp
    Event/Button/Repeat.cpp
    Event/Common/Time.cpp
    Event/Cursor/PositionX.cpp
    Event/Cursor/PositionY.cpp
    Event/Data.cpp
    Event/Field.cpp
    ShutdownHandler.cpp
    UnsupportedOperationException.cpp
    CommandLine/InvalidCommandLineOption.cpp
)