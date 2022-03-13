add_program_linux(
    TARGET ${LIBRARY_NAME}
    SRCS
    CommandLine/ParserLinux.cpp
    ../../../checkinput/src/EventDevice.cpp
    ../../../checkinput/src/EventDeviceLister.cpp
    ShutdownHandlerLinux.cpp
    XInputHandler.cpp
    EventTransformerLinux.cpp
    XInputEvent.cpp
)