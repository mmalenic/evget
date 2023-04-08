add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_DEPENDENCIES
    SRCS
    XInputEvent.cpp
    XSetMaskRefresh.cpp
    XSetMaskCore.cpp
    XWrapperX11.cpp
)
add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_TOUCH_SUPPORT
    SRCS XSetMaskTouch.cpp
)