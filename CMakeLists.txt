cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 23)

# x-release-please-start-version
project(
    evget
    VERSION 0.1.0
    DESCRIPTION "record input device events"
    HOMEPAGE_URL "https://github.com/mmalenic/evget"
)
# x-release-please-end

if(NOT DEFINED toolbelt_SOURCE_DIR)
    include(FetchContent)
    FetchContent_Declare(
            toolbelt
            GIT_REPOSITORY https://github.com/mmalenic/cmake-toolbelt
            GIT_TAG v0.3.2
    )
    FetchContent_MakeAvailable(toolbelt)
endif()

list(APPEND CMAKE_MODULE_PATH "${toolbelt_SOURCE_DIR}/src")
include(toolbelt)

if(EVLIST_RUN_CLANG_TIDY)
    if(DEFINED EVLIST_CLANG_TIDY_EXECUTABLE)
        set(CMAKE_CXX_CLANG_TIDY "${EVLIST_CLANG_TIDY_EXECUTABLE}")
    else()
        set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    endif()
endif()

add_subdirectory(evgetcore)

toolbelt_check_includes(
        VAR
        EVGETX11_HAS_FEATURE
        INCLUDES
        X11/Xlib.h
        X11/extensions/XInput.h
        X11/extensions/XInput2.h
        X11/Xutil.h
        xorg/xserver-properties.h
        LANGUAGE CXX
)
if(EVGETX11_HAS_FEATURE)
    add_subdirectory(evgetx11)
endif()

if(EVGET_BUILD_BIN)
    add_executable(${PROJECT_NAME} evgetcore/src/main.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGET_LIBRARY_NAME})
    if(EVGETX11_HAS_FEATURE)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGETX11_LIBRARY_NAME})
    endif()
endif()

if(EVGET_INSTALL_BIN AND EVGET_BUILD_BIN)
    install(TARGETS ${PROJECT_NAME})
endif()
