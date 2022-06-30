add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_DEPENDENCIES
    SRCS
    EventTransformerX11.cpp
    ParserLinux.cpp
    XInputEvent.cpp
    XInputHandler.cpp
    XEventSwitchCore.cpp
    XEventSwitch.cpp
    XEventSwitchPointer.cpp
    XSetMaskRefresh.cpp
    XSetMaskCore.cpp
    XWrapperX11.cpp
    XDeviceDeleter.cpp
    XEventCookieDeleter.cpp
    XWrapper.cpp
)
add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_TOUCH_SUPPORT
    SRCS XEventSwitchTouch.cpp XSetMaskTouch.cpp
)