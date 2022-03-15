add_program_linux(
    TARGET ${EVGETX11_LIBRARY_NAME}
    SRCS
    ParserLinux.cpp
    ShutdownHandlerLinux.cpp
    XInputHandler.cpp
    EventTransformerLinux.cpp
    XInputEvent.cpp
)