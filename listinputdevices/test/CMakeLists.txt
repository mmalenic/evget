add_program(
    TARGET ${LISTINPUTDEVICES_TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
    SRCS
    EventDeviceListerTest.cpp
    EventDeviceTest.cpp
    utils/ListInputDevicesTestUtils.h
    utils/ListInputDevicesTestUtils.cpp
)

program_dependencies(TARGET ${LISTINPUTDEVICES_TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
program_dependencies(TARGET ${LISTINPUTDEVICES_TEST_EXECUTABLE_NAME} DEP PUBLIC ${LISTINPUTDEVICES_LIBRARY_NAME} ONLY_LINK)

include(GoogleTest)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
gtest_discover_tests(${LISTINPUTDEVICES_TEST_EXECUTABLE_NAME})