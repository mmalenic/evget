add_program(
    TARGET ${LISTINPUTDEVICES_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    FEATURE_REQUIRES fcntl.h linux/input.h
    FEATURE_CHECK LISTINPUTDEVICES_HAS_DEPENDENCIES
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    SRCS
    InputDeviceListerTest.cpp
    InputDeviceTest.cpp
    utils/ListInputDevicesTestUtils.h
    utils/ListInputDevicesTestUtils.cpp
)

link_testing(LISTINPUTDEVICES_TEST_EXECUTABLE_NAME LISTINPUTDEVICES_LIBRARY_NAME)