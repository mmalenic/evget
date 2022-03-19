add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_REQUIRES X11/Xlib.h X11/extensions/XInput.h X11/extensions/XInput2.h X11/Xutil.h xorg/xserver-properties.h
    FEATURE_CHECK EVGETX11_HAS_DEPENDENCIES
    SRCS
    EventTransformerLinux.cpp
    ParserLinux.cpp
    ShutdownHandlerLinux.cpp
    XInputEvent.cpp
    XInputHandler.cpp
    CoreXEventSwitch.cpp
)