add_program(
    TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    SRCS
    CommandLineParserLinuxTest.cpp
    ShutdownHandlerLinuxTest.cpp
    XInputHandlerTest.cpp
)

program_dependencies(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
program_dependencies(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} DEP PUBLIC ${EVGETX11_LIBRARY_NAME} ONLY_LINK)
program_dependencies(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} DEP PUBLIC X11 COMPONENTS Xtst)

include(GoogleTest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
gtest_discover_tests(${EVGETX11_TEST_EXECUTABLE_NAME})