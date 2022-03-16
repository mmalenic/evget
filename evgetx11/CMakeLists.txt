set_and_get_variable(EVGETX11_LIBRARY_NAME libevgetx11)
set_and_get_variable(EVGETX11_TEST_EXECUTABLE_NAME libevgetx11test)

add_program(
    TARGET ${EVGETX11_LIBRARY_NAME} MAKE_LIBRARY
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
set_target_properties(${EVGETX11_LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(src)

program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC X11 COMPONENTS X11 Xi Xkb)
program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC fmt 8.0.1)
program_dependencies(TARGET ${EVGETX11_LIBRARY_NAME} DEP PUBLIC spdlog 1.9.2)

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
    add_program(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE)

    add_subdirectory(test)
    program_dependencies(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
    program_dependencies(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} DEP PUBLIC ${EVGETX11_LIBRARY_NAME} ONLY_LINK)
endif()