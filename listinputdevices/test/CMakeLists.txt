set(TEST_EXECUTABLE_NAME listinputdevicestest)

add_executable(
    ${TEST_EXECUTABLE_NAME}
    InputDeviceListerTest.cpp
    InputDeviceTest.cpp
    utils/ListInputDevicesTestUtils.h
    utils/ListInputDevicesTestUtils.cpp
)
target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC utils)

setup_testing(${TEST_EXECUTABLE_NAME} ${PROJECT_NAME})