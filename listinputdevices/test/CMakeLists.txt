add_program(
    TARGET ${LISTINPUTDEVICES_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    SRCS
    EventDeviceListerTest.cpp
    EventDeviceTest.cpp
    utils/ListInputDevicesTestUtils.h
    utils/ListInputDevicesTestUtils.cpp
)

link_testing(LISTINPUTDEVICES_TEST_EXECUTABLE_NAME LISTINPUTDEVICES_LIBRARY_NAME)