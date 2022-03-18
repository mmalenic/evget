add_program(
    TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    SRCS
    AbstractOptionTest.cpp
    OptionFlagTest.cpp
    OptionTest.cpp
    OptionValidatedTest.cpp
    ParserTest.cpp
    utils/CliOptionTestUtils.h
)

link_testing(CLIOPTION_TEST_EXECUTABLE_NAME CLIOPTION_LIBRARY_NAME)