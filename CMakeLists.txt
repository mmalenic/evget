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
    # Fetch content from this repo.
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

add_executable(${PROJECT_NAME} evgetcore/src/main.cpp)

#add_subdirectory(util)
#target_link_libraries(${PROJECT_NAME} PUBLIC util)
#set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)

#add_subdirectory(async)
#target_link_libraries(${PROJECT_NAME} PUBLIC async)
#
#add_subdirectory(database)
#target_link_libraries(${PROJECT_NAME} PUBLIC database)
#
#add_subdirectory(clioption)
#target_link_libraries(${PROJECT_NAME} PUBLIC clioption)

add_subdirectory(evgetcore)
target_link_libraries(${PROJECT_NAME} PRIVATE ${EVGET_LIBRARY_NAME})

#target_link_libraries(${PROJECT_NAME} PUBLIC evgetcore)

#toolbelt_check_includes(
#    VAR
#    EVGETX11_HAS_FEATURE
#    INCLUDES
#    X11/Xlib.h
#    X11/extensions/XInput.h
#    X11/extensions/XInput2.h
#    X11/Xutil.h
#    xorg/xserver-properties.h
#    LANGUAGE CXX
#)
#if(EVGETX11_HAS_FEATURE)
#    add_subdirectory(evgetx11)
#    target_link_libraries(${PROJECT_NAME} PUBLIC evgetx11)
#endif()
