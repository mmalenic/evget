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
set(EXECUTABLE_NAME "evget")
set(TEST_EXECUTABLE_NAME "evget_test")

option(DOWNLOAD_DEPENDENCIES "Download dependencies if they are missing." ON)

INCLUDE(CheckIncludeFiles)
INCLUDE(CTest)

# Adds program dependencies by using find_package and target_link_libraries.
function(program_dependencies)
    set(options PLATFORM_GUARD)
    set(one_value_args TARGET)
    set(multi_value_args FIND_DEPS LINK_DEPS)
    cmake_parse_arguments(PROGRAM_DEPENDENCIES "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if (PROGRAM_DEPENDENCIES_PLATFORM_GUARD)
        find_package(${PROGRAM_DEPENDENCIES_FIND_DEPS})
        if (TARGET PROGRAM_DEPENDENCIES_LINK_DEPS)
            message("Found package ${PROGRAM_DEPENDENCIES_FIND_DEPS}, linked to ${PROGRAM_DEPENDENCIES_TARGET}")
            target_link_libraries(${PROGRAM_DEPENDENCIES_TARGET} ${PROGRAM_DEPENDENCIES_LINK_DEPS})
        endif()
    endif()
endfunction()

# Adds a program executable, library, or source files to an existing targets, 
# and optionally includes libraries.
function(add_program)
    set(options MAKE_EXECUTABLE MAKE_LIBRARY PLATFORM_GUARD)
    set(one_value_args TARGET)
    set(multi_value_args SRCS INCLUDE_DIRS PLATFORM_REQUIRES)
    cmake_parse_arguments(ADD_PROGRAM "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
    
    if (DEFINED ADD_PROGRAM_PLATFORM_REQUIRES)
        CHECK_INCLUDE_FILES("${ADD_PROGRAM_PLATFORM_REQUIRES}" REQUIRES)
    else()
        set(REQUIRES TRUE)
    endif()

    if (ADD_PROGRAM_PLATFORM_GUARD AND REQUIRES)
        if (DEFINED ADD_PROGRAM_TARGET)

            if (DEFINED ADD_PROGRAM_SRCS)
                if (ADD_PROGRAM_MAKE_EXECUTABLE)
                    message("Added executable ${ADD_PROGRAM_TARGET}")
                    add_executable(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_SRCS})
                elseif (ADD_PROGRAM_MAKE_LIBRARY)
                    message("Added library ${ADD_PROGRAM_TARGET}")
                    add_executable(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_SRCS})
                else()
                    message("Added sources to ${ADD_PROGRAM_TARGET}")
                    target_sources(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_SRCS})
                endif()
            endif()

            if (DEFINED ADD_PROGRAM_FOR_INCLUDE_DIRS)
                message("Included directories for ${ADD_PROGRAM_TARGET}")
                target_include_directories(${ADD_PROGRAM_FOR_TARGET} ${ADD_PROGRAM_FOR_INCLUDE_DIRS})
            endif()

        endif()
    endif()
endfunction()

if (DOWNLOAD_DEPENDENCIES)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
    endif()

    include(${CMAKE_BINARY_DIR}/conan.cmake)
    conan_cmake_autodetect(settings)

    conan_add_remote(
        NAME conancenter
        URL https://center.conan.io
        VERIFY_SSL True
    )
    conan_cmake_install(
        PATH_OR_REFERENCE ${CMAKE_SOURCE_DIR}
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings}
    )
endif()

# Common functionality.
add_program(
    TARGET ${EXECUTABLE_NAME} MAKE_EXECUTABLE
    PLATFORM_GUARD TRUE
    INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    SRCS
    # Headers
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
    # Sources
    src/CommandLine/CommandLine.cpp
    src/EventData/Field.cpp
    src/EventData/EventData.cpp
    src/EventData/MouseMove.cpp
    src/ShutdownHandler.cpp
    src/UnsupportedOperationException.cpp
    src/InvalidCommandLineOption.cpp
    src/CommandLine/CommandLineOption.cpp
)

# Linux functionality.
add_program(
    TARGET evget MAKE_EXECUTABLEW
    PLATFORM_GUARD UNIX AND NOT APPLE
    PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
    INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/platform/linux
    ${PROJECT_SOURCE_DIR}/src/platform/linux
    SRCS
    # Headers
    # Sources
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