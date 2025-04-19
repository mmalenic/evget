cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 23)
# x-release-please-start-version
project(
    evgetx11
    VERSION 0.1.0
    DESCRIPTION "record input device events using X11"
    HOMEPAGE_URL "https://github.com/mmalenic/evget"
)
# x-release-please-end

if(NOT DEFINED toolbelt_SOURCE_DIR)
    include(FetchContent)
    # Fetch content from this repo.
    FetchContent_Declare(
            toolbelt
            GIT_REPOSITORY https://github.com/mmalenic/cmake-toolbelt
            GIT_TAG v0.3.1
    )
    FetchContent_MakeAvailable(toolbelt)
endif()

list(APPEND CMAKE_MODULE_PATH "${toolbelt_SOURCE_DIR}/src")
include(toolbelt)

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
    XInputEvent.cpp
    XSetMaskRefresh.cpp
    XSetMaskCore.cpp
    XWrapperX11.cpp
    EventLoopX11.cpp
    XEventSwitch.cpp
    XEventSwitchPointerKey.cpp
    XInputHandler.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

toolbelt_check_symbol(VAR EVGETX11_HAS_TOUCH_SUPPORT SYMBOL XI_TouchBegin FILES X11/extensions/XInput2.h)
if(EVGETX11_HAS_TOUCH_SUPPORT)
    target_sources(${PROJECT_NAME} PUBLIC include/evgetx11/XEventSwitchTouch.h include/evgetx11/XSetMaskTouch.h XEventSwitchTouch.cpp XSetMaskTouch.cpp)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC evgetcore)

toolbelt_add_dep(${PROJECT_NAME} Boost VISIBILITY PUBLIC LINK_COMPONENTS headers)
toolbelt_add_dep(${PROJECT_NAME} fmt VISIBILITY PUBLIC)
toolbelt_add_dep(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
toolbelt_add_dep(
    ${PROJECT_NAME}
    X11
    VISIBILITY
    PUBLIC
    FIND_PACKAGE_ARGS COMPONENTS Xi Xkb
)

if(BUILD_TESTING)
    set(TEST_EXECUTABLE_NAME evgetx11test)

    add_executable(
            ${TEST_EXECUTABLE_NAME} EventTransformerX11Test.cpp XEventSwitchTest.cpp
            XEventSwitchPointerKeyTest.cpp utils/EvgetX11TestUtils.h utils/Evgetx11TestUtils.cpp
    )
    target_include_directories(${TEST_EXECUTABLE_NAME} PUBLIC utils)

    if(EVGETX11_HAS_TOUCH_SUPPORT)
        target_sources(${TEST_EXECUTABLE_NAME} PRIVATE XEventSwitchTouchTest.cpp)
    endif()

    toolbelt_setup_gtest(${TEST_EXECUTABLE_NAME})
endif()
