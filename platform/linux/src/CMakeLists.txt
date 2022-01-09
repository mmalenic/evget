add_program(
    TARGET ${LIBRARY_NAME}
    PLATFORM UNIX AND NOT APPLE
    PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
    PLATFORM_CHECK Linux
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    SRCS
    EventDevice.cpp
    EventDeviceLister.cpp
    ShutdownHandlerLinux.cpp
)