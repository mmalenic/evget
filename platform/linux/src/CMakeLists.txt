add_program_linux(
    TARGET ${LIBRARY_NAME}
    SRCS
    CommandLine/ParserLinux.cpp
    EventDevice.cpp
    EventDeviceLister.cpp
    ShutdownHandlerLinux.cpp
)