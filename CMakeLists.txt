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
set(TEST_EXECUTABLE_NAME "evget_test")

option(DOWNLOAD_DEPENDENCIES "Download dependencies if they are missing." ON)

INCLUDE(CheckIncludeFiles)
INCLUDE(CTest)

# Checks the platform, returning the a result equivalent to true for the current platform.
function(platform_check)
    set(one_value_args RESULT PLATFORM_GUARD)
    set(multi_value_args PLATFORM_REQUIRES)
    cmake_parse_arguments(PLATFORM_CHECK "" "${one_value_args}" "${multi_value_args}" ${ARGN})
    
    if (NOT DEFINED PLATFORM_CHECK_PLATFORM_GUARD OR "${PLATFORM_CHECK_PLATFORM_GUARD}" OR "${${PLATFORM_CHECK_PLATFORM_GUARD}}")
        if (NOT DEFINED PLATFORM_CHECK_PLATFORM_REQUIRES OR "${PLATFORM_CHECK_PLATFORM_REQUIRES}" STREQUAL "")
            set(${PLATFORM_CHECK_RESULT} TRUE PARENT_SCOPE)
        else()
            CHECK_INCLUDE_FILES("${PLATFORM_CHECK_PLATFORM_REQUIRES}" REQUIRES)
            if (REQUIRES)
                set(${PLATFORM_CHECK_RESULT} TRUE PARENT_SCOPE)
            endif()
        endif()
    endif()

    unset(REQUIRES CACHE)
endfunction()

# Adds program dependencies by using find_package and target_link_libraries.
function(program_dependencies)
    set(one_value_args TARGET PLATFORM_GUARD)
    set(multi_value_args DEP LINK PLATFORM_REQUIRES)
    cmake_parse_arguments(PROGRAM_DEPENDENCIES "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    platform_check(PLATFORM_GUARD ${PROGRAM_DEPENDENCIES_PLATFORM_GUARD} PLATFORM_REQUIRES ${PROGRAM_DEPENDENCIES_PLATFORM_REQUIRES} RESULT GUARD)
    if (GUARD)
        # Try to guess link.
        if (NOT DEFINED PROGRAM_DEPENDENCIES_LINK)
            set(PROGRAM_DEPENDENCIES_LINK ${PROGRAM_DEPENDENCIES_DEP}::${PROGRAM_DEPENDENCIES_DEP})
        endif()

        find_package(${PROGRAM_DEPENDENCIES_DEP})
        if (TARGET ${PROGRAM_DEPENDENCIES_LINK})
            message("Found package ${PROGRAM_DEPENDENCIES_LINK}, linked with ${PROGRAM_DEPENDENCIES_TARGET}")
            target_link_libraries(${PROGRAM_DEPENDENCIES_TARGET} ${PROGRAM_DEPENDENCIES_LINK_DEP})
        endif()
    endif()
endfunction()

# Adds a program executable, library, or source files to an existing targets, 
# and optionally includes libraries.
function(add_program)
    set(options MAKE_EXECUTABLE MAKE_LIBRARY)
    set(one_value_args TARGET PLATFORM_GUARD)
    set(multi_value_args SRCS INCLUDE_DIRS PLATFORM_REQUIRES)
    cmake_parse_arguments(ADD_PROGRAM "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
    
    platform_check(PLATFORM_GUARD ${ADD_PROGRAM_PLATFORM_GUARD} PLATFORM_REQUIRES ${ADD_PROGRAM_PLATFORM_REQUIRES} RESULT GUARD)
    if (GUARD)
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
    TARGET ${CMAKE_PROJECT_NAME} MAKE_EXECUTABLE
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
program_dependencies(TARGET ${CMAKE_PROJECT_NAME} DEP fmt)

# Linux functionality.
add_program(
    TARGET ${CMAKE_PROJECT_NAME}
    PLATFORM_GUARD UNIX AND NOT APPLE
    PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
    INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/platform/linux
    ${PROJECT_SOURCE_DIR}/src/platform/linux
    SRCS
    # Headers
    include/platform/linux/CommandLine/CommandLineLinux.h
    include/platform/linux/EventDevice.h
    include/platform/linux/EventDeviceLister.h
    include/platform/linux/ShutdownHandlerLinux.h
    # Sources
    src/platform/linux/CommandLine/CommandLineLinux.cpp
    src/platform/linux/EventDevice.cpp
    src/platform/linux/EventDeviceLister.cpp
    src/platform/linux/ShutdownHandlerLinux.cpp
)

# OSX functionality
add_program(
    TARGET ${CMAKE_PROJECT_NAME}
    PLATFORM_GUARD APPLE
    PLATFORM_REQUIRES
    INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/platform/osx
    ${PROJECT_SOURCE_DIR}/src/platform/osx
    SRCS
    # Headers
    # Sources
)

# Windows functionality
add_program(
    TARGET ${CMAKE_PROJECT_NAME}
    PLATFORM_GUARD WIN32
    PLATFORM_REQUIRES
    INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/platform/win32_64
    ${PROJECT_SOURCE_DIR}/src/platform/win32_64
    SRCS
    # Headers
    # Sources
)

if (BUILD_TESTING)
    # Common tests.
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test
        SRCS
        test/EventData/EventDataTest.cpp
        test/EventData/FieldTest.cpp
        test/CommandLine/CommandLineTest.cpp
        test/CommandLine/CommandLineOptionTest.cpp
        test/CommandLine/CommandLineOptionBuilderTest.cpp
    )

    # Linux tests.
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME}
        PLATFORM_GUARD UNIX AND NOT APPLE
        PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test/platform/linux
        SRCS
        test/platform/linux/CommandLine/CommandLineLinuxTest.cpp
        test/platform/linux/EventDeviceListerTest.cpp
        test/platform/linux/EventDeviceTest.cpp
        test/platform/linux/ShutdownHandlerLinuxTest.cpp
    )

    # OSX tests
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME}
        PLATFORM_GUARD APPLE
        PLATFORM_REQUIRES
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test/platform/osx
        SRCS
    )

    # Windows tests
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME}
        PLATFORM_GUARD WIN32
        PLATFORM_REQUIRES
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test/platform/win32_64
        SRCS
    )

    include(GoogleTest)
    gtest_discover_tests(${TEST_EXECUTABLE_NAME})
endif()