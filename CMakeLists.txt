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

cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(evget)

add_executable(${PROJECT_NAME} evgetcore/src/main.cpp)

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

add_subdirectory(evwatch)

check_includes(RETURN_VAR LISTINPUTDEVICES_HAS_FEATURE REQUIRES fcntl.h linux/input.h)
if(LISTINPUTDEVICES_HAS_FEATURE)
    add_subdirectory(listinputdevices)
endif()

check_includes(RETURN_VAR EVWATCHSIG_HAS_FEATURE REQUIRES signal.h)
if(EVWATCHSIG_HAS_FEATURE)
    add_subdirectory(evwatchsig)
endif()
