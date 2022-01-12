add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    SRCS
    CommandLineParserTest.cpp
    Event/EventDataTest.cpp
    Event/FieldTest.cpp
)

add_subdirectory(TestUtilities)