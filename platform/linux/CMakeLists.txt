add_program_linux(
    TARGET ${LIBRARY_NAME}
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    include/evget/CommandLine/ParserLinux.h
    include/evget/EventDevice.h
    include/evget/EventDeviceLister.h
    include/evget/ShutdownHandlerLinux.h
    include/evget/SystemEventLoopLinux.h
    include/evget/XInputHandler.h
)

add_subdirectory(src)

program_dependencies(TARGET ${LIBRARY_NAME} DEP PUBLIC X11 COMPONENTS X11 Xi)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()