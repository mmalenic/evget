add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_DEPENDENCIES
    SRCS
    EventTransformerLinux.cpp
    ParserLinux.cpp
    ShutdownHandlerLinux.cpp
    XInputEvent.cpp
    XInputHandler.cpp
    CoreXEventSwitch.cpp
    XEventSwitch.cpp
    XSetMaskRefresh.cpp
    XSetMaskCore.cpp
    XWrapperX11.cpp
    XDeviceDeleter.cpp
    XEventCookieDeleter.cpp
)
add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_TOUCH_SUPPORT
    SRCS TouchXEventSwitch.cpp XSetMaskTouch.cpp
)
add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_BARRIER_SUPPORT
    SRCS XEventSwitchBarrier.cpp XSetMaskBarrier.cpp
)