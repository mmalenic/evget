add_program_linux(
    TARGET ${TEST_EXECUTABLE_NAME}
    SRCS
    CommandLine/CommandLineParserLinuxTest.cpp
    EventDeviceListerTest.cpp
    EventDeviceTest.cpp
    ShutdownHandlerLinuxTest.cpp
)