add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    SRCS
    CommandLine/CommandLineOptionBaseTest.cpp
    CommandLine/CommandLineOptionFlagTest.cpp
    CommandLine/CommandLineOptionTest.cpp
    CommandLine/CommandLineOptionValidatedTest.cpp
    CommandLine/CommandLineParserTest.cpp
    Event/Common/TimeTest.cpp
    Event/Button/CharacterTest.cpp
    Event/Button/PressTest.cpp
    Event/Button/ReleaseTest.cpp
    Event/Button/RepeatTest.cpp
    Event/EventDataTest.cpp
    Event/FieldTest.cpp
)

add_subdirectory(TestUtils)