# MIT License
#
# Copyright (c) 2021 Marko Malenic
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 23)

# x-release-please-start-version
project(
    evget
    VERSION 1.0.4
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
            GIT_TAG v0.3.1
    )
    FetchContent_MakeAvailable(toolbelt)
endif()

list(APPEND CMAKE_MODULE_PATH "${toolbelt_SOURCE_DIR}/src")
include(toolbelt)

add_executable(${PROJECT_NAME} evgetcore/src/main.cpp)

add_subdirectory(util)
target_link_libraries(${PROJECT_NAME} PUBLIC util)
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(async)
target_link_libraries(${PROJECT_NAME} PUBLIC async)

add_subdirectory(database)
target_link_libraries(${PROJECT_NAME} PUBLIC database)

add_subdirectory(clioption)
target_link_libraries(${PROJECT_NAME} PUBLIC clioption)

add_subdirectory(evgetcore)
target_link_libraries(${PROJECT_NAME} PUBLIC evgetcore)

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
    add_subdirectory(evgetx11)
    target_link_libraries(${PROJECT_NAME} PUBLIC evgetx11)
endif()
