target_sources(
    ${PROJECT_NAME}
    PRIVATE XInputEvent.cpp
            XSetMaskRefresh.cpp
            XSetMaskCore.cpp
            XWrapperX11.cpp
            EventLoopX11.cpp
            XEventSwitch.cpp
            XEventSwitchPointerKey.cpp
            XInputHandler.cpp
)

if(EVGETX11_HAS_TOUCH_SUPPORT)
    target_sources(${PROJECT_NAME} PRIVATE XEventSwitchTouch.cpp XSetMaskTouch.cpp)
endif()
