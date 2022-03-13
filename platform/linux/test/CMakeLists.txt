add_program_linux(
    TARGET ${TEST_EXECUTABLE_NAME}
    SRCS
    CommandLine/CommandLineParserLinuxTest.cpp
    ../../../checkinput/test/EventDeviceListerTest.cpp
    ../../../checkinput/test/EventDeviceTest.cpp
    ShutdownHandlerLinuxTest.cpp
    XInputHandlerTest.cpp
)

program_dependencies(TARGET ${TEST_EXECUTABLE_NAME} DEP PUBLIC X11 COMPONENTS Xtst)