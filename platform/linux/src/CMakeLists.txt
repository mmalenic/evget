add_program_linux(
    TARGET ${LIBRARY_NAME}
    SRCS
    CommandLine/ParserLinux.cpp
    ../../../listinputdevices/src/InputDevice.cpp
    ../../../listinputdevices/src/InputDeviceLister.cpp
    ShutdownHandlerLinux.cpp
    XInputHandler.cpp
    EventTransformerLinux.cpp
    XInputEvent.cpp
)