add_program(
    TARGET ${TEST_EXECUTABLE_NAME}
    PLATFORM UNIX AND NOT APPLE
    PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
    PLATFORM_CHECK Linux
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    SRCS
    EventDeviceListerTest.cpp
    EventDeviceTest.cpp
    ShutdownHandlerLinuxTest.cpp
)