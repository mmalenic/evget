target_sources(
    ${PROJECT_NAME}
    PRIVATE
    XInputEvent.cpp
    XSetMaskRefresh.cpp
    XSetMaskCore.cpp
    XWrapperX11.cpp
)

if (EVGETX11_HAS_TOUCH_SUPPORT)
    target_sources(
        ${PROJECT_NAME}
        PRIVATE
        XSetMaskTouch.cpp
    )
endif ()