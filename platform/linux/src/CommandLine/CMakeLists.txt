add_program(
    TARGET ${LIBRARY_NAME}
    PLATFORM UNIX AND NOT APPLE
    PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
    PLATFORM_CHECK Linux
    SRCS
    ParserLinux.cpp
)