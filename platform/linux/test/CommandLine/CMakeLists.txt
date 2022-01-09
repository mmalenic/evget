add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    PLATFORM UNIX AND NOT APPLE
    PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
    PLATFORM_CHECK Linux
    SRCS
    CommandLineParserLinuxTest.cpp
)