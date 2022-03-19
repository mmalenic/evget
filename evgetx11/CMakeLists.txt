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
    include/evgetx11/ShutdownHandlerLinux.h
    include/evgetx11/SystemEventLoopLinux.h
    include/evgetx11/XInputHandler.h
    include/evgetx11/EventTransformerLinux.h
    include/evgetx11/XInputEvent.h
)
add_subdirectory(src)

program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC X11 COMPONENTS X11 Xi Xkb)
program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC fmt 8.0.1)
program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC spdlog 1.9.2)

add_library_and_link(EVGETX11_LIBRARY_NAME CLIOPTION_LIBRARY_NAME "")
add_library_and_link(EVGETX11_LIBRARY_NAME EVGETCORE_LIBRARY_NAME "")

find_symbols(
    TARGET ${EVGETX11_LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGET_X11_HAS_TOUCH_SUPPORT
    SYMBOLS XI_TouchBegin XI_TouchUpdate XI_TouchEnd XI_TouchOwnership XI_RawTouchBegin XI_RawTouchUpdate XI_RawTouchEnd
    FILES X11/extensions/XInput2.h
)
find_symbols(
    TARGET ${EVGETX11_LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGET_X11_HAS_BARRIER_SUPPORT
    SYMBOLS XI_BarrierHit XI_BarrierLeave
    FILES X11/extensions/XInput2.h
)
find_symbols(
    TARGET ${EVGETX11_LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGET_X11_HAS_GESTURE_SUPPORT
    SYMBOLS XI_GesturePinchBegin XI_GesturePinchUpdate XI_GesturePinchEnd XI_GestureSwipeBegin XI_GestureSwipeUpdate XI_GestureSwipeEnd
    FILES X11/extensions/XInput2.h
)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()