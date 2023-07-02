set(TEST_EXECUTABLE_NAME clioptiontest)

add_executable(
    ${TEST_EXECUTABLE_NAME}
    AbstractOptionTest.cpp
    OptionFlagTest.cpp
    OptionTest.cpp
    OptionValidatedTest.cpp
    ParserTest.cpp
    utils/CliOptionTestUtils.h
)
target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC utils)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})