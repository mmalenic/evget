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
    include/evget/EventTransformerLinux.h
    include/evget/XInputEvent.h
)

add_subdirectory(src)

program_dependencies(TARGET ${LIBRARY_NAME} DEP PUBLIC X11 COMPONENTS X11 Xi Xkb)

#find_symbols(
#    SYMBOLS XI_TouchBegin XI_TouchUpdate XI_TouchEnd XI_TouchOwnership XI_RawTouchBegin XI_RawTouchUpdate XI_RawTouchEnd
#    FILES X11/extensions/XInput2.h
#    RESULT HAS_XI22
#)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()