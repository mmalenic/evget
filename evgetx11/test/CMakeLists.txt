add_program_linux(
    TARGET ${EVGETX11_TEST_EXECUTABLE_NAME}
    SRCS
    CommandLineParserLinuxTest.cpp
    ShutdownHandlerLinuxTest.cpp
    XInputHandlerTest.cpp
)

program_dependencies(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} DEP PUBLIC X11 COMPONENTS Xtst)