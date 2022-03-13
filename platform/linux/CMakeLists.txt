add_program_linux(
    TARGET ${LIBRARY_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/evget/CommandLine/ParserLinux.h
    ../../checkinput/include/checkinput/EventDevice.h
    ../../checkinput/include/checkinput/EventDeviceLister.h
    include/evget/ShutdownHandlerLinux.h
    include/evget/SystemEventLoopLinux.h
    include/evget/XInputHandler.h
    include/evget/EventTransformerLinux.h
    include/evget/XInputEvent.h
)

add_subdirectory(src)

program_dependencies(TARGET ${LIBRARY_NAME} DEP PUBLIC X11 COMPONENTS X11 Xi Xkb)

find_symbols(
    TARGET ${LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGET_X11_HAS_TOUCH_SUPPORT
    SYMBOLS XI_TouchBegin XI_TouchUpdate XI_TouchEnd XI_TouchOwnership XI_RawTouchBegin XI_RawTouchUpdate XI_RawTouchEnd
    FILES X11/extensions/XInput2.h
)
find_symbols(
    TARGET ${LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGET_X11_HAS_BARRIER_SUPPORT
    SYMBOLS XI_BarrierHit XI_BarrierLeave
    FILES X11/extensions/XInput2.h
)
find_symbols(
    TARGET ${LIBRARY_NAME} VISIBILITY PUBLIC NAME EVGET_X11_HAS_GESTURE_SUPPORT
    SYMBOLS XI_GesturePinchBegin XI_GesturePinchUpdate XI_GesturePinchEnd XI_GestureSwipeBegin XI_GestureSwipeUpdate XI_GestureSwipeEnd
    FILES X11/extensions/XInput2.h
)


if (BUILD_TESTING)
    add_subdirectory(test)
endif()