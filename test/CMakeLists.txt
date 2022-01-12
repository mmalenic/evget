add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    SRCS
    CommandLineParserTest.cpp
    ../Event/test/EventDataTest.cpp
    ../Event/test/FieldTest.cpp
)

add_subdirectory(TestUtilities)