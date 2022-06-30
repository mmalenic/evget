set_and_get_variable(EVGETX11_LIBRARY_NAME libevgetx11)
set_and_get_variable(EVGETX11_TEST_EXECUTABLE_NAME libevgetx11test)

add_program(
    TARGET ${EVGETX11_LIBRARY_NAME} MAKE_LIBRARY
    FEATURE_REQUIRES X11/Xlib.h X11/extensions/XInput.h X11/extensions/XInput2.h X11/Xutil.h xorg/xserver-properties.h
    FEATURE_CHECK EVGETX11_HAS_DEPENDENCIES
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/evgetx11/ParserLinux.h
    include/evgetx11/SystemEventLoopX11.h
    include/evgetx11/XInputHandler.h
    include/evgetx11/EventTransformerX11.h
    include/evgetx11/XInputEvent.h
    include/evgetx11/XEventSwitch.h
    include/evgetx11/XEventSwitchCore.h
    include/evgetx11/XEventSwitchPointer.h
    include/evgetx11/XSetMask.h
    include/evgetx11/XSetMaskRefresh.h
    include/evgetx11/XSetMaskCore.h
    include/evgetx11/XWrapperX11.h
    include/evgetx11/XWrapper.h
    include/evgetx11/XDeviceDeleter.h
    include/evgetx11/XEventCookieDeleter.h
)

find_symbols(
    TARGET ${EVGETX11_LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGETX11_HAS_TOUCH_SUPPORT
    SYMBOLS XI_TouchBegin XI_TouchUpdate XI_TouchEnd XI_TouchOwnership XI_RawTouchBegin XI_RawTouchUpdate XI_RawTouchEnd
    FILES X11/extensions/XInput2.h
)
add_program(
    TARGET ${EVGETX11_LIBRARY_NAME}
    FEATURE_CHECK EVGETX11_HAS_TOUCH_SUPPORT
    SRCS include/evgetx11/XEventSwitchTouch.h include/evgetx11/XSetMaskTouch.h
)

find_symbols(
    TARGET ${EVGETX11_LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGETX11_HAS_GESTURE_SUPPORT
    SYMBOLS XI_GesturePinchBegin XI_GesturePinchUpdate XI_GesturePinchEnd XI_GestureSwipeBegin XI_GestureSwipeUpdate XI_GestureSwipeEnd
    FILES X11/extensions/XInput2.h
)

add_subdirectory(src)

program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC X11 COMPONENTS X11 Xi Xkb)
program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC fmt ${FMT_VERSION})
program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC spdlog ${SPDLOG_VERSION})

add_library_and_link(EVGETX11_LIBRARY_NAME CLIOPTION_LIBRARY_NAME "")
add_library_and_link(EVGETX11_LIBRARY_NAME EVGETCORE_LIBRARY_NAME "")

if (BUILD_TESTING)
    add_subdirectory(test)
endif()