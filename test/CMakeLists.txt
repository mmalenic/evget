add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    SRCS
    CommandLine/CommandLineParserTest.cpp
    Event/EventDataTest.cpp
    Event/FieldTest.cpp
)

add_subdirectory(TestUtilities)