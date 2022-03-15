add_program(
    TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME}
    SRCS
    AbstractOptionTest.cpp
    OptionFlagTest.cpp
    OptionTest.cpp
    OptionValidatedTest.cpp
    ParserTest.cpp
    utils/CliOptionTestUtils.h
)