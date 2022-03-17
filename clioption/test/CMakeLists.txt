add_program(
    TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    SRCS
    AbstractOptionTest.cpp
    OptionFlagTest.cpp
    OptionTest.cpp
    OptionValidatedTest.cpp
    ParserTest.cpp
    utils/CliOptionTestUtils.h
)

program_dependencies(TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
program_dependencies(TARGET ${CLIOPTION_TEST_EXECUTABLE_NAME} DEP PUBLIC ${CLIOPTION_LIBRARY_NAME} ONLY_LINK)

include(GoogleTest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
gtest_discover_tests(${CLIOPTION_TEST_EXECUTABLE_NAME})