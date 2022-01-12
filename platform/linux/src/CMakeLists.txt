add_program_linux(
    TARGET ${LIBRARY_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    SRCS
    EventDevice.cpp
    EventDeviceLister.cpp
    ShutdownHandlerLinux.cpp
)