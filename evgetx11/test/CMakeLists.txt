add_program(
    TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    FEATURE_REQUIRES X11/Xlib.h X11/extensions/XInput.h X11/extensions/XInput2.h X11/Xutil.h xorg/xserver-properties.h
    FEATURE_CHECK EVGETX11_HAS_DEPENDENCIES
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    SRCS
    EventTransformerX11Test.cpp
    XEventSwitchTest.cpp
    XEventSwitchCoreTest.cpp
    utils/EvgetX11TestUtils.h
)
program_dependencies(TARGET ${EVGETX11_TEST_EXECUTABLE_NAME} DEP PUBLIC X11 COMPONENTS Xtst)

link_testing(EVGETX11_TEST_EXECUTABLE_NAME EVGETX11_LIBRARY_NAME)