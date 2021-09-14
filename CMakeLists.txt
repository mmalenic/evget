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
project(evget)

set(CMAKE_CXX_STANDARD 20)

option(DOWNLOAD_DEPENDENCIES "Download dependencies if they are missing." ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

function(add_program_for)
    set(options MAKE_EXECUTABLE)
    set(one_value_args TARGET)
    set(multi_value_args SRCS DEPENDENCIES INCLUDE_DIRS)
    cmake_parse_arguments(ADD_PROGRAM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if (DEFINED ADD_PROGRAM_FOR_TARGET)

        if (DEFINED ADD_PROGRAM_FOR_SRCS)
            if (ADD_PROGRAM_FOR_MAKE_EXECUTABLE)
                add_executable(ADD_PROGRAM_FOR_TARGET ADD_PROGRAM_FOR_SRCS)
            else()
                target_sources(ADD_PROGRAM_FOR_TARGET ADD_PROGRAM_FOR_SRCS)
            endif()
        endif()

        if (DEFINED ADD_PROGRAM_FOR_DEPENDENCIES)
            target_link_libraries(ADD_PROGRAM_FOR_TARGET ADD_PROGRAM_FOR_DEPENDENCIES)
        endif()

        if (DEFINED ADD_PROGRAM_FOR_INCLUDE_DIRS)
            target_include_directories(ADD_PROGRAM_FOR_TARGET ADD_PROGRAM_FOR_INCLUDE_DIRS)
        endif()

    endif()
endfunction()

function(add_program)
    set(options MAKE_EXECUTABLE PLATFORM_GUARD)
    set(one_value_args TARGET TEST_TARGET)
    set(multi_value_args SRCS DEPENDENCIES INCLUDE_DIRS TEST_SRCS TEST_DEPENDENCIES TEST_INCLUDE_DIRS)
    cmake_parse_arguments(ADD_PROGRAM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (ADD_PROGRAM_PLATFORM_GUARD OR NOT DEFINED ADD_PROGRAM_PLATFORM_GUARD)
        add_program_for(
            MAKE_EXECUTABLE ADD_PROGRAM_MAKE_EXECUTABLE
            TARGET ADD_PROGRAM_TARGET
            SRCS ADD_PROGRAM_SRCS DEPENDENCIES ADD_PROGRAM_DEPENDENCIES INCLUDE_DIRS ADD_PROGRAM_INCLUDE_DIRS
        )

        if (BUILD_TESTING AND DEFINED ADD_PROGRAM_TEST_TARGET)
            add_program_for(
                MAKE_EXECUTABLE ADD_PROGRAM_MAKE_EXECUTABLE
                TARGET ADD_PROGRAM_TEST_TARGET
                SRCS ADD_PROGRAM_TEST_SRCS DEPENDENCIES ADD_PROGRAM_TEST_DEPENDENCIES INCLUDE_DIRS ADD_PROGRAM_TEST_INCLUDE_DIRS
            )
            include(GoogleTest)
            gtest_discover_tests(ADD_PROGRAM_TEST_TARGET)
        endif()

    endif()
endfunction()

set(HEADERS_COMMON
    include/Task.h
    include/CommandLine/CommandLine.h
    include/EventTransformer.h
    include/EventData/EventData.h
    include/EventHandler.h
    include/SystemEvent.h
    include/SystemEventLoop.h
    include/ShutdownHandler.h
    include/Storage.h
    include/EventData/MouseMove.h
    include/EventListener.h
    include/EventData/Field.h
    include/UnsupportedOperationException.h
    include/CommandLine/CommandLineOption.h
    include/InvalidCommandLineOption.h
    include/CommandLine/CommandLineOptionBuilder.h
    )

set(SOURCES_COMMON
    src/CommandLine/CommandLine.cpp
    src/EventData/Field.cpp
    src/EventData/EventData.cpp
    src/EventData/MouseMove.cpp
    src/ShutdownHandler.cpp
    src/UnsupportedOperationException.cpp
    src/InvalidCommandLineOption.cpp
    src/CommandLine/CommandLineOption.cpp
    )

if (BUILD_TESTING)
    include_directories(
        ${PROJECT_SOURCE_DIR}/test
    )
    set(TESTS_COMMON
        test/EventData/EventDataTest.cpp
        test/EventData/FieldTest.cpp
        test/CommandLine/CommandLineTest.cpp
        test/CommandLine/CommandLineOptionTest.cpp
        test/CommandLine/CommandLineOptionBuilderTest.cpp
    )
endif()

if (UNIX AND NOT APPLE)
    include_directories(
        include/platform/linux
        src/platform/linux
    )

    set(HEADERS
        include/platform/linux/CommandLine/CommandLineLinux.h
        include/platform/linux/EventDevice.h
        include/platform/linux/EventDeviceLister.h
        include/platform/linux/SystemEventLoopLinux.h
        include/platform/linux/ShutdownHandlerLinux.h
    )

    set(SOURCES
        src/platform/linux/CommandLine/CommandLineLinux.cpp
        src/platform/linux/EventDevice.cpp
        src/platform/linux/EventDeviceLister.cpp
        src/platform/linux/ShutdownHandlerLinux.cpp
    )

    if (BUILD_TESTING)
        include_directories(
            test/platform/linux
        )

        set(TESTS
            test/platform/linux/CommandLine/CommandLineLinuxTest.cpp
            test/platform/linux/EventDeviceListerTest.cpp
            test/platform/linux/EventDeviceTest.cpp
            test/platform/linux/ShutdownHandlerLinuxTest.cpp
        )
    endif()
endif()

if (APPLE)
    include_directories(
        ${PROJECT_SOURCE_DIR}/include/osx
        ${PROJECT_SOURCE_DIR}/src/osx
    )

    set(HEADERS
    )

    set(SOURCES
    )

    if(BUILD_TESTING)
        include_directories(
            ${PROJECT_SOURCE_DIR}/test/osx
        )

        set(TESTS
        )
    endif()
endif()

if (WIN32)
    include_directories(
        ${PROJECT_SOURCE_DIR}/include/win32_64
        ${PROJECT_SOURCE_DIR}/src/win32_64
    )

    set(HEADERS
    )

    set(SOURCES
    )

    if(BUILD_TESTING)
        include_directories(
            ${PROJECT_SOURCE_DIR}/test/win32_64
        )

        set(TESTS
        )
    endif()
endif()

if (DOWNLOAD_DEPENDENCIES)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
    endif()

    include(${CMAKE_BINARY_DIR}/conan.cmake)
    conan_cmake_autodetect(settings)
    conan_cmake_install(
        PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}
        BUILD missing
        REMOTE conan-center
        SETTINGS ${settings}
    )
endif()

add_executable(evget
    src/main.cpp
    ${HEADERS_COMMON}
    ${SOURCES_COMMON}
    ${HEADERS}
    ${SOURCES}
)
target_link_libraries(evget CONAN_PKG::boost CONAN_PKG::spdlog)

include(CTest)

if (BUILD_TESTING)
    add_executable(Test
        ${HEADERS_COMMON}
        ${SOURCES_COMMON}
        ${HEADERS}
        ${SOURCES}
        ${TESTS_COMMON}
        ${TESTS}
    )
    target_link_libraries(Test CONAN_PKG::gtest CONAN_PKG::boost CONAN_PKG::spdlog)

    include(GoogleTest)
    gtest_discover_tests(Test)
endif()

