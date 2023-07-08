cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)
include("${CMAKE_SOURCE_DIR}/utils.cmake")

check_includes(
    RETURN_VAR
    EVGETX11_HAS_FEATURE
    REQUIRES
    X11/Xlib.h
    X11/extensions/XInput.h
    X11/extensions/XInput2.h
    X11/Xutil.h
    xorg/xserver-properties.h
)

if(EVGETX11_HAS_FEATURE)
    project(evgetx11)

    add_library(
        ${PROJECT_NAME}
        include/evgetx11/EventLoopX11.h
        include/evgetx11/XInputHandler.h
        include/evgetx11/EventTransformerX11.h
        include/evgetx11/XInputEvent.h
        include/evgetx11/XEventSwitchPointerKey.h
        include/evgetx11/XEventSwitch.h
        include/evgetx11/XSetMask.h
        include/evgetx11/XSetMaskRefresh.h
        include/evgetx11/XSetMaskCore.h
        include/evgetx11/XWrapperX11.h
        include/evgetx11/XWrapper.h
        include/evgetx11/DeleterWithDisplay.h
    )
    target_include_directories(${PROJECT_NAME} PUBLIC include)

    check_symbol(RETURN_VAR EVGETX11_HAS_TOUCH_SUPPORT SYMBOL XI_TouchBegin FILES X11/extensions/XInput2.h)
    if(EVGETX11_HAS_TOUCH_SUPPORT)
        target_sources(${PROJECT_NAME} PUBLIC include/evgetx11/XEventSwitchTouch.h include/evgetx11/XSetMaskTouch.h)
    endif()

    add_subdirectory(src)

    target_link_libraries(${PROJECT_NAME} PUBLIC evgetcore)
    target_link_libraries(${PROJECT_NAME} PUBLIC clioption)

    program_dependencies(${PROJECT_NAME} fmt VISIBILITY PUBLIC)
    program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
    program_dependencies(
        ${PROJECT_NAME}
        X11
        COMPONENTS
        Xi
        Xkb
        VISIBILITY
        PUBLIC
    )

    if(BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
