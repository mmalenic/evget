# MIT License
#
# Copyright (c) 2021 Marko Malenic
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.19)
project(input-event-recorder)

set(CMAKE_CXX_STANDARD 20)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

if(UNIX AND NOT APPLE)
    include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/linux
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/linux
    )

    set(HEADERS
        include/CommandLine.h
        include/linux/CommandLineLinux.h
        include/linux/EventDevice.h
        include/linux/EventDeviceLister.h
    )
    set(SOURCES
        src/CommandLine.cpp
        src/linux/CommandLineLinux.cpp
        src/linux/EventDevice.cpp
        src/linux/EventDeviceLister.cpp
    )

    if(BUILD_TESTING)
        include_directories(
            ${PROJECT_SOURCE_DIR}/test
            ${PROJECT_SOURCE_DIR}/test/linux
        )

        set(TESTS
            test/linux/CommandLineLinuxTest.cpp
            test/linux/EventDeviceTest.cpp
        )
    endif()
endif()

if(WIN32)
    include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/win32_64
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/win32_64
    )

    set(HEADERS
    )
    set(SOURCES
    )

    if(BUILD_TESTING)
        include_directories(
            ${PROJECT_SOURCE_DIR}/test
            ${PROJECT_SOURCE_DIR}/test/win32_64
        )

        set(TESTS
        )
    endif()
endif()

find_package(Boost 1.71.0 COMPONENTS program_options filesystem)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(input-event-recorder
        src/main.cpp
        ${HEADERS}
        ${SOURCES}
    )
    target_link_libraries(input-event-recorder ${Boost_LIBRARIES})
endif()

include(CTest)

if(BUILD_TESTING)
    include_directories(
        ${PROJECT_SOURCE_DIR}/test
        ${PROJECT_SOURCE_DIR}/test/linux
        ${PROJECT_SOURCE_DIR}/test/win32_64
    )

    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG e2239ee6043f73722e7aa812a459f54a28552929 # release 1.11.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(CommandLineLinuxTest
        ${HEADERS}
        ${SOURCES}
        ${TESTS}
    )
    target_link_libraries(
        CommandLineLinuxTest gtest_main ${Boost_LIBRARIES}
    )

    include(GoogleTest)
    gtest_discover_tests(CommandLineLinuxTest)
endif()

