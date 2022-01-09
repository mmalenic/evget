add_program_linux(
    TARGET ${LIBRARY_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/evget
    SRCS
    include/evget/CommandLine/ParserLinux.h
    include/evget/EventDevice.h
    include/evget/EventDeviceLister.h
    include/evget/ShutdownHandlerLinux.h
    include/evget/SystemEventLoopLinux.h
)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()