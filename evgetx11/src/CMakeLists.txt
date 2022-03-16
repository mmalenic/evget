add_program_linux(
    TARGET ${EVGETX11_LIBRARY_NAME}
    SRCS
    EventTransformerLinux.cpp
    ParserLinux.cpp
    ShutdownHandlerLinux.cpp
    XInputEvent.cpp
    XInputHandler.cpp
)