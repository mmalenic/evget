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
set(TEST_EXECUTABLE_NAME "evget-test")

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
# DEP can specify arguments: visibility, dependency name, and version.
# COMPONENTS should specify arguments in pairs of: visibility, and component name.
# If COMPONENTS is present, visibility from DEP is ignored.
function(program_dependencies)
    set(options NOT_REQUIRED FIND_ALL ONLY_LINK)
    set(one_value_args PLATFORM_GUARD)
    set(multi_value_args TARGET DEP PLATFORM_REQUIRES COMPONENTS)
    cmake_parse_arguments(PROGRAM_DEPENDENCIES "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
    
    platform_check(PLATFORM_GUARD ${PROGRAM_DEPENDENCIES_PLATFORM_GUARD} PLATFORM_REQUIRES ${PROGRAM_DEPENDENCIES_PLATFORM_REQUIRES} RESULT GUARD)
    if (GUARD AND DEFINED PROGRAM_DEPENDENCIES_DEP)
        list(LENGTH PROGRAM_DEPENDENCIES_DEP DEP_LENGTH)
        if (${DEP_LENGTH} EQUAL 3)
            list(GET PROGRAM_DEPENDENCIES_DEP 0 DEPENDENCY_VISIBILITY)
            list(GET PROGRAM_DEPENDENCIES_DEP 1 DEPENDENCY_NAME)
            list(GET PROGRAM_DEPENDENCIES_DEP 2 DEPENDENCY_VERSION)
        elseif (${DEP_LENGTH} EQUAL 2)
            list(GET PROGRAM_DEPENDENCIES_DEP 0 DEPENDENCY_NAME)
            if ("${DEPENDENCY_NAME}" STREQUAL "PUBLIC" OR "${DEPENDENCY_NAME}" STREQUAL "PRIVATE" OR "${DEPENDENCY_NAME}" STREQUAL "INTERFACE")
                list(GET PROGRAM_DEPENDENCIES_DEP 0 DEPENDENCY_VISIBILITY)
                list(GET PROGRAM_DEPENDENCIES_DEP 1 DEPENDENCY_NAME)
            else()
                list(GET PROGRAM_DEPENDENCIES_DEP 1 DEPENDENCY_VERSION)
            endif()
        elseif (${DEP_LENGTH} EQUAL 1)
            list(GET PROGRAM_DEPENDENCIES_DEP 0 DEPENDENCY_NAME)
        else()
            message(FATAL_ERROR "Required argument to DEP unspecified.")
        endif()

        if (PROGRAM_DEPENDENCIES_ONLY_LINK AND TARGET ${DEPENDENCY_NAME})
            message(STATUS "Linking ${DEPENDENCY_NAME} with ${PROGRAM_DEPENDENCIES_TARGET}.")
            target_link_libraries(${PROGRAM_DEPENDENCIES_TARGET} ${DEPENDENCY_VISIBILITY} ${DEPENDENCY_NAME})
            return()
        endif()

        if (DEFINED PROGRAM_DEPENDENCIES_COMPONENTS)
            list(LENGTH PROGRAM_DEPENDENCIES_COMPONENTS LIST_LENGTH)
            math(EXPR LOOP "${LIST_LENGTH} - 1")
            foreach(INDEX RANGE ${LOOP})
                list(GET PROGRAM_DEPENDENCIES_COMPONENTS ${INDEX} COMP)
                if ("${COMP}" STREQUAL "PUBLIC" OR "${COMP}" STREQUAL "PRIVATE" OR "${COMP}" STREQUAL "INTERFACE")
                    list(APPEND COMPONENTS ${COMP})
                endif()
            endforeach()
        endif()
           
        if (PROGRAM_DEPENDENCIES_NOT_REQUIRED AND (NOT DEFINED PROGRAM_DEPENDENCIES_COMPONENTS OR PROGRAM_DEPENDENCIES_FIND_ALL))
            find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION})
        elseif (PROGRAM_DEPENDENCIES_NOT_REQUIRED AND DEFINED PROGRAM_DEPENDENCIES_COMPONENTS)
            find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION} COMPONENTS ${COMPONENTS})
        elseif (NOT DEFINED PROGRAM_DEPENDENCIES_COMPONENTS OR PROGRAM_DEPENDENCIES_FIND_ALL)
            find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION} REQUIRED)
        else()
            find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION} REQUIRED COMPONENTS ${COMPONENTS})
        endif()
        
        if (DEFINED DEPENDENCY_VERSION)
            message(STATUS "Found package ${DEPENDENCY_NAME} with version ${DEPENDENCY_VERSION}.")
        else()
            message(STATUS "Found package ${DEPENDENCY_NAME}.")
        endif()

        # Try to guess components
        if (NOT DEFINED PROGRAM_DEPENDENCIES_COMPONENTS AND TARGET ${DEPENDENCY_NAME}::${DEPENDENCY_NAME})
            message(STATUS "Found target ${DEPENDENCY_NAME}::${DEPENDENCY_NAME}, linking with ${PROGRAM_DEPENDENCIES_TARGET}")
            target_link_libraries(${PROGRAM_DEPENDENCIES_TARGET} ${DEPENDENCY_VISIBILITY} ${DEPENDENCY_NAME}::${DEPENDENCY_NAME})
        elseif (DEFINED PROGRAM_DEPENDENCIES_COMPONENTS)
            list(LENGTH PROGRAM_DEPENDENCIES_COMPONENTS COMPONENTS_LENGTH)
            math(EXPR LOOP "${COMPONENTS_LENGTH} - 1")
            foreach(INDEX RANGE 0 ${LOOP})
                list(GET PROGRAM_DEPENDENCIES_COMPONENTS ${INDEX} COMP)
                if ("${COMP}" STREQUAL "PUBLIC" OR "${COMP}" STREQUAL "PRIVATE" OR "${COMP}" STREQUAL "INTERFACE")
                    set(DEPENDENCY_VISIBILITY ${COMP})
                elseif (TARGET ${DEPENDENCY_NAME}::${COMP})
                    message(STATUS "Found target ${DEPENDENCY_NAME}::${COMP}, linking with ${PROGRAM_DEPENDENCIES_TARGET}.")
                    target_link_libraries(${PROGRAM_DEPENDENCIES_TARGET} ${DEPENDENCY_VISIBILITY} ${DEPENDENCY_NAME}::${COMP})
                endif()
            endforeach()
        else()
            message(FATAL_ERROR "Undefined combination of arguments.")
        endif()
    endif()
endfunction()

# Adds a program executable, library, or source files to an existing targets, 
# and optionally includes libraries.
function(add_program)
    set(options MAKE_EXECUTABLE MAKE_LIBRARY)
    set(one_value_args TARGET PLATFORM_GUARD INCLUDE_VISIBILITY)
    set(multi_value_args SRCS INCLUDE_DIRS PLATFORM_REQUIRES)
    cmake_parse_arguments(ADD_PROGRAM "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
    
    platform_check(PLATFORM_GUARD ${ADD_PROGRAM_PLATFORM_GUARD} PLATFORM_REQUIRES ${ADD_PROGRAM_PLATFORM_REQUIRES} RESULT GUARD)
    if (GUARD)
        if (DEFINED ADD_PROGRAM_TARGET)
            if (DEFINED ADD_PROGRAM_SRCS)
                if (ADD_PROGRAM_MAKE_EXECUTABLE)
                    message(STATUS "Adding executable ${ADD_PROGRAM_TARGET}.")
                    add_executable(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_SRCS})
                elseif (ADD_PROGRAM_MAKE_LIBRARY)
                    message(STATUS "Adding library ${ADD_PROGRAM_TARGET}.")
                    add_library(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_SRCS})
                else()
                    message(STATUS "Adding sources to ${ADD_PROGRAM_TARGET}.")
                    target_sources(${ADD_PROGRAM_TARGET} PRIVATE ${ADD_PROGRAM_SRCS})
                endif()
            endif()

            if (DEFINED ADD_PROGRAM_INCLUDE_DIRS)
                if (DEFINED ADD_PROGRAM_INCLUDE_VISIBILITY)
                    message(STATUS "Including ${ADD_PROGRAM_INCLUDE_VISIBILITY} directories: ${ADD_PROGRAM_INCLUDE_DIRS} for ${ADD_PROGRAM_TARGET}.")
                    target_include_directories(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_INCLUDE_VISIBILITY} ${ADD_PROGRAM_INCLUDE_DIRS})
                else()
                    message(STATUS "Including PUBLIC directories: ${ADD_PROGRAM_INCLUDE_DIRS} for ${ADD_PROGRAM_TARGET}.")
                    target_include_directories(${ADD_PROGRAM_TARGET} PUBLIC ${ADD_PROGRAM_INCLUDE_DIRS})
                endif()
            endif()
        endif()
    endif()
endfunction()

if (DOWNLOAD_DEPENDENCIES)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan.")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
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
    set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
    set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})
endif()

# Common functionality.
add_program(
    TARGET ${CMAKE_PROJECT_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    SRCS
    # Headers
    include/Task.h
    include/CommandLine/Parser.h
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
    include/CommandLine/Option.h
    include/InvalidCommandLineOption.h
    include/CommandLine/OptionBuilder.h
    include/CommandLine/OptionBase.h
    include/CommandLine/OptionValidated.h
    include/CommandLine/OptionFlag.h
    # Sources
    src/CommandLine/OptionFlag.cpp
    src/CommandLine/Parser.cpp
    src/EventData/Field.cpp
    src/EventData/EventData.cpp
    src/EventData/MouseMove.cpp
    src/ShutdownHandler.cpp
    src/UnsupportedOperationException.cpp
    src/InvalidCommandLineOption.cpp
)
program_dependencies(TARGET ${CMAKE_PROJECT_NAME} DEP PUBLIC fmt 8.0.1)
program_dependencies(TARGET ${CMAKE_PROJECT_NAME} DEP PUBLIC spdlog 1.9.2)
program_dependencies(TARGET ${CMAKE_PROJECT_NAME} DEP PUBLIC Boost 1.77 FIND_ALL COMPONENTS program_options headers)

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
    include/platform/linux/CommandLine/ParserLinux.h
    include/platform/linux/EventDevice.h
    include/platform/linux/EventDeviceLister.h
    include/platform/linux/ShutdownHandlerLinux.h
    # Sources
    src/platform/linux/CommandLine/ParserLinux.cpp
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

# Make executable.
add_program(
    TARGET ${CMAKE_PROJECT_NAME}-bin MAKE_EXECUTABLE
    SRCS
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)
program_dependencies(TARGET ${CMAKE_PROJECT_NAME}-bin DEP PUBLIC ${CMAKE_PROJECT_NAME} ONLY_LINK)
set_target_properties(${CMAKE_PROJECT_NAME}-bin PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME})
message(STATUS "Renamed executable ${CMAKE_PROJECT_NAME}-bin to ${CMAKE_PROJECT_NAME}.")

if (BUILD_TESTING)
    # Common tests.
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/include
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/src
        SRCS
        test/EventData/EventDataTest.cpp
        test/EventData/FieldTest.cpp
        test/CommandLine/CommandLineTest.cpp
        test/CommandLine/CommandLineOptionTest.cpp
        test/CommandLine/CommandLineOptionBaseTest.cpp
        test/CommandLine/CommandLineOptionValidatedTest.cpp
        test/CommandLine/CommandLineOptionFlagTest.cpp
        # Util Headers
        test/TestUtilities/include/CommandLine/CommandLineTestUtilities.h
        # Util Sources
        test/TestUtilities/src/CommandLine/CommandLineTestUtilities.cpp
    )
    program_dependencies(TARGET ${TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest)

    # Linux tests.
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME}
        PLATFORM_GUARD UNIX AND NOT APPLE
        PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test/platform/linux
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/include/platform/linux
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/src/platform/linux
        SRCS
        test/platform/linux/CommandLine/CommandLineLinuxTest.cpp
        test/platform/linux/EventDeviceListerTest.cpp
        test/platform/linux/EventDeviceTest.cpp
        test/platform/linux/ShutdownHandlerLinuxTest.cpp
        # Util Headers
        # Util Sources
    )

    # OSX tests
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME}
        PLATFORM_GUARD APPLE
        PLATFORM_REQUIRES
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test/platform/osx
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/include/platform/osx
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/src/platform/osx
        SRCS
        # Util Headers
        # Util Sources
    )

    # Windows tests
    add_program(
        TARGET ${TEST_EXECUTABLE_NAME}
        PLATFORM_GUARD WIN32
        PLATFORM_REQUIRES
        INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/test/platform/win32_64
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/include/platform/win32_64
        ${PROJECT_SOURCE_DIR}/test/TestUtilities/src/platform/win32_64
        SRCS
        # Util Headers
        # Util Sources
    )
    program_dependencies(TARGET ${TEST_EXECUTABLE_NAME} DEP PUBLIC ${CMAKE_PROJECT_NAME} ONLY_LINK)

    include(GoogleTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    gtest_discover_tests(${TEST_EXECUTABLE_NAME})
endif()