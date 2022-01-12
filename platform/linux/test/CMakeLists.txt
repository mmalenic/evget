add_program_linux(
    TARGET ${TEST_EXECUTABLE_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    SRCS
    EventDeviceListerTest.cpp
    EventDeviceTest.cpp
    ShutdownHandlerLinuxTest.cpp
)