add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_DEPENDENCIES
    SRCS
    ParserLinux.cpp
    XInputEvent.cpp
    XInputHandler.cpp
    XEventSwitchCore.cpp
        XDeviceRefresh.cpp
    XEventSwitchPointer.cpp
    XSetMaskRefresh.cpp
    XSetMaskCore.cpp
    XWrapperX11.cpp
    XWrapper.cpp
)
add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_TOUCH_SUPPORT
    SRCS XEventSwitchTouch.cpp XSetMaskTouch.cpp
)