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
set(LIBRARY_NAME libevget)
set(TEST_EXECUTABLE_NAME evgettest)

option(DOWNLOAD_DEPENDENCIES "Download dependencies if they are missing." ON)

INCLUDE(CheckIncludeFiles)
INCLUDE(CTest)

# Checks the platform, returning the result true for the current platform.
function(platform_check)
    set(one_value_args RESULT PLATFORM)
    set(multi_value_args PLATFORM_REQUIRES)
    cmake_parse_arguments(PLATFORM_CHECK "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    get_property(CHECK GLOBAL PROPERTY ${PLATFORM_CHECK_RESULT})
    if (NOT "${CHECK}" STREQUAL "")
        message(STATUS "Skipping platform check for ${PLATFORM_CHECK_RESULT}, already checked.")
        return()
    endif()

    if (NOT DEFINED PLATFORM_CHECK_PLATFORM OR "${PLATFORM_CHECK_PLATFORM}" OR "${${PLATFORM_CHECK_PLATFORM}}")
        message("${${PLATFORM_CHECK_PLATFORM_REQUIRES}}")
        if (NOT DEFINED PLATFORM_CHECK_PLATFORM_REQUIRES OR PLATFORM_CHECK_PLATFORM_REQUIRES STREQUAL "")
            if (DEFINED PLATFORM_CHECK_RESULT)
                set_property(GLOBAL PROPERTY ${PLATFORM_CHECK_RESULT} TRUE)
            endif()
        else()
            check_include_files("${PLATFORM_CHECK_PLATFORM_REQUIRES}" REQUIRES)
            set_property(GLOBAL PROPERTY ${PLATFORM_CHECK_RESULT} ${REQUIRES})
        endif()
    endif()

    unset(REQUIRES CACHE)
endfunction()

# Links a found target.
function(link_target)
    set(multi_value_args TARGET DEP VISIBILITY)
    cmake_parse_arguments(LINK_TARGET "" "" "${multi_value_args}" ${ARGN})

    message(STATUS "Found target ${LINK_TARGET_DEP}, linking with ${LINK_TARGET_TARGET} with ${LINK_TARGET_VISIBILITY} visibility.")
    target_link_libraries(${LINK_TARGET_TARGET} ${LINK_TARGET_VISIBILITY} ${LINK_TARGET_DEP})
endfunction()

# Links a target if found, with relaxed case.
function(link_case_relaxed)
    set(one_value_args RESULT)
    set(multi_value_args TARGET DEP VISIBILITY)
    cmake_parse_arguments(LINK_CASE_RELAXED "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    string(TOLOWER ${LINK_CASE_RELAXED_DEP} LOWER_DEP)
    string(TOUPPER ${LINK_CASE_RELAXED_DEP} UPPER_DEP)
    if (TARGET ${LINK_CASE_RELAXED_DEP})
        link_target(TARGET ${LINK_CASE_RELAXED_TARGET} VISIBILITY ${LINK_CASE_RELAXED_VISIBILITY} DEP ${LINK_CASE_RELAXED_DEP})
        set(${LINK_CASE_RELAXED_RESULT} TRUE PARENT_SCOPE)
    elseif (TARGET ${LOWER_DEP})
        link_target(TARGET ${LINK_CASE_RELAXED_TARGET} VISIBILITY ${LINK_CASE_RELAXED_VISIBILITY} DEP ${LOWER_DEP})
        set(${LINK_CASE_RELAXED_RESULT} TRUE PARENT_SCOPE)
    elseif (TARGET ${UPPER_DEP})
        link_target(TARGET ${LINK_CASE_RELAXED_TARGET} VISIBILITY ${LINK_CASE_RELAXED_VISIBILITY} DEP ${UPPER_DEP})
        set(${LINK_CASE_RELAXED_RESULT} TRUE PARENT_SCOPE)
    endif()
endfunction()

# Adds program dependencies by using find_package and target_link_libraries.
# DEP can specify arguments: visibility, dependency name, and version.
# COMPONENTS should specify arguments in pairs of: visibility, and component name.
# If COMPONENTS is present, visibility from DEP is ignored.
function(program_dependencies)
    set(options NOT_REQUIRED FIND_ALL ONLY_LINK)
    set(one_value_args PLATFORM PLATFORM_CHECK)
    set(multi_value_args TARGET DEP PLATFORM_REQUIRES COMPONENTS)
    cmake_parse_arguments(PROGRAM_DEPENDENCIES "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if (DEFINED PROGRAM_DEPENDENCIES_PLATFORM_CHECK)
        platform_check(PLATFORM ${PROGRAM_DEPENDENCIES_PLATFORM} PLATFORM_REQUIRES ${PROGRAM_DEPENDENCIES_PLATFORM_REQUIRES} RESULT ${PROGRAM_DEPENDENCIES_PLATFORM_CHECK})
        get_property(GUARD GLOBAL PROPERTY ${PROGRAM_DEPENDENCIES_PLATFORM_CHECK})
    endif()

    if ((NOT DEFINED PROGRAM_DEPENDENCIES_PLATFORM_CHECK OR GUARD) AND DEFINED PROGRAM_DEPENDENCIES_DEP)
        list(LENGTH PROGRAM_DEPENDENCIES_DEP DEP_LENGTH)
        if (${DEP_LENGTH} EQUAL 3)
            list(GET PROGRAM_DEPENDENCIES_DEP 0 DEPENDENCY_VISIBILITY)
            list(GET PROGRAM_DEPENDENCIES_DEP 1 DEPENDENCY_NAME)
            list(GET PROGRAM_DEPENDENCIES_DEP 2 DEPENDENCY_VERSION)
        elseif (${DEP_LENGTH} EQUAL 2)
            list(GET PROGRAM_DEPENDENCIES_DEP 0 DEPENDENCY_NAME)
            if (DEPENDENCY_NAME STREQUAL "PUBLIC" OR DEPENDENCY_NAME STREQUAL "PRIVATE" OR DEPENDENCY_NAME STREQUAL "INTERFACE")
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
                if (COMP STREQUAL "PUBLIC" OR COMP STREQUAL "PRIVATE" OR COMP STREQUAL "INTERFACE")
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
        if (NOT DEFINED PROGRAM_DEPENDENCIES_COMPONENTS)
            message("HERE")
            link_case_relaxed(TARGET ${PROGRAM_DEPENDENCIES_TARGET} DEP ${DEPENDENCY_NAME}::${DEPENDENCY_NAME} VISIBILITY ${DEPENDENCY_VISIBILITY})
        elseif (DEFINED PROGRAM_DEPENDENCIES_COMPONENTS)
            list(LENGTH PROGRAM_DEPENDENCIES_COMPONENTS COMPONENTS_LENGTH)
            math(EXPR LOOP "${COMPONENTS_LENGTH} - 1")
            foreach(INDEX RANGE 0 ${LOOP})
                list(GET PROGRAM_DEPENDENCIES_COMPONENTS ${INDEX} COMP)
                if (COMP STREQUAL "PUBLIC" OR COMP STREQUAL "PRIVATE" OR COMP STREQUAL "INTERFACE")
                    set(DEPENDENCY_VISIBILITY ${COMP})
                else()
                    link_case_relaxed(TARGET ${PROGRAM_DEPENDENCIES_TARGET} DEP ${DEPENDENCY_NAME}::${COMP} VISIBILITY ${DEPENDENCY_VISIBILITY} RESULT LINKED)
                    if (NOT LINKED)
                        link_case_relaxed(TARGET ${PROGRAM_DEPENDENCIES_TARGET} DEP ${COMP} VISIBILITY ${DEPENDENCY_VISIBILITY})
                    endif()
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
    set(one_value_args TARGET PLATFORM PLATFORM_CHECK INCLUDE_VISIBILITY)
    set(multi_value_args SRCS INCLUDE_DIRS PLATFORM_REQUIRES)
    cmake_parse_arguments(ADD_PROGRAM "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if (DEFINED ADD_PROGRAM_PLATFORM_CHECK)
        platform_check(PLATFORM ${ADD_PROGRAM_PLATFORM} PLATFORM_REQUIRES ${ADD_PROGRAM_PLATFORM_REQUIRES} RESULT ${ADD_PROGRAM_PLATFORM_CHECK})
        get_property(GUARD GLOBAL PROPERTY ${ADD_PROGRAM_PLATFORM_CHECK})
    endif()

    if (NOT DEFINED ADD_PROGRAM_PLATFORM_CHECK OR GUARD)
        if (DEFINED ADD_PROGRAM_TARGET)
            if (DEFINED ADD_PROGRAM_SRCS)
                if (ADD_PROGRAM_MAKE_EXECUTABLE)
                    message(STATUS "Creating executable ${ADD_PROGRAM_TARGET}.")
                    add_executable(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_SRCS})
                elseif (ADD_PROGRAM_MAKE_LIBRARY)
                    message(STATUS "Creating library ${ADD_PROGRAM_TARGET}.")
                    add_library(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_SRCS})
                else()
                    message(STATUS "Adding sources to ${ADD_PROGRAM_TARGET}.")
                    target_sources(${ADD_PROGRAM_TARGET} PRIVATE ${ADD_PROGRAM_SRCS})
                endif()
            elseif(ADD_PROGRAM_MAKE_EXECUTABLE)
                message(STATUS "Creating empty executable ${ADD_PROGRAM_TARGET}.")
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.cpp "")
                add_executable(${ADD_PROGRAM_TARGET} ${CMAKE_CURRENT_BINARY_DIR}/null.cpp)
            endif()

            if (DEFINED ADD_PROGRAM_INCLUDE_DIRS)
                if (NOT DEFINED ADD_PROGRAM_INCLUDE_VISIBILITY)
                    set(ADD_PROGRAM_INCLUDE_VISIBILITY PUBLIC)
                endif()
                message(STATUS "Including ${ADD_PROGRAM_INCLUDE_VISIBILITY} directories: ${ADD_PROGRAM_INCLUDE_DIRS} for ${ADD_PROGRAM_TARGET}.")
                target_include_directories(${ADD_PROGRAM_TARGET} ${ADD_PROGRAM_INCLUDE_VISIBILITY} ${ADD_PROGRAM_INCLUDE_DIRS})
            endif()
        endif()
    endif()
endfunction()

# Add linux specific code.
function(add_program_linux)
    set(options MAKE_EXECUTABLE MAKE_LIBRARY)
    set(one_value_args TARGET INCLUDE_VISIBILITY)
    set(multi_value_args SRCS INCLUDE_DIRS)
    cmake_parse_arguments(ADD_PROGRAM_LINUX "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    add_program(
        TARGET ${ADD_PROGRAM_LINUX_TARGET} ${ADD_PROGRAM_LINUX_MAKE_EXECUTABLE} ${ADD_PROGRAM_LINUX_MAKE_LIBRARY}
        PLATFORM UNIX AND NOT APPLE
        PLATFORM_REQUIRES fcntl.h linux/input.h sys/utsname.h
        PLATFORM_CHECK Linux
        SRCS ${ADD_PROGRAM_LINUX_SRCS}
        INCLUDE_DIRS ${ADD_PROGRAM_LINUX_INCLUDE_DIRS}
        INCLUDE_VISIBILITY ${ADD_PROGRAM_LINUX_INCLUDE_VISIBILITY}
    )
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
    TARGET ${LIBRARY_NAME} MAKE_LIBRARY
    INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SRCS
    # Headers
    include/evget/Task.h
    include/evget/CommandLine/Parser.h
    include/evget/EventTransformer.h
    include/evget/Event/Data.h
    include/evget/EventHandler.h
    include/evget/SystemEvent.h
    include/evget/SystemEventLoop.h
    include/evget/ShutdownHandler.h
    include/evget/Storage.h
    include/evget/Event/MouseMove.h
    include/evget/Event/Cursor/PositionX.h
    include/evget/Event/Cursor/PositionY.h
    include/evget/EventListener.h
    include/evget/Event/Field.h
    include/evget/UnsupportedOperationException.h
    include/evget/CommandLine/Option.h
    include/evget/InvalidCommandLineOption.h
    include/evget/CommandLine/OptionBuilder.h
    include/evget/CommandLine/OptionBase.h
    include/evget/CommandLine/OptionValidated.h
    include/evget/CommandLine/OptionFlag.h
)
add_subdirectory(src)
add_subdirectory(platform/linux)

program_dependencies(TARGET ${LIBRARY_NAME} DEP PUBLIC fmt 8.0.1)
program_dependencies(TARGET ${LIBRARY_NAME} DEP PUBLIC spdlog 1.9.2)
program_dependencies(TARGET ${LIBRARY_NAME} DEP PUBLIC Boost 1.77 FIND_ALL COMPONENTS program_options headers)

# Make executable.
add_program(
    TARGET ${CMAKE_PROJECT_NAME} MAKE_EXECUTABLE
    SRCS
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)
program_dependencies(TARGET ${CMAKE_PROJECT_NAME} DEP PUBLIC ${LIBRARY_NAME} ONLY_LINK)

if (BUILD_TESTING)
    add_program(TARGET ${TEST_EXECUTABLE_NAME} MAKE_EXECUTABLE)

    add_subdirectory(test)
    program_dependencies(TARGET ${TEST_EXECUTABLE_NAME} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
    program_dependencies(TARGET ${TEST_EXECUTABLE_NAME} DEP PUBLIC ${LIBRARY_NAME} ONLY_LINK)

    include(GoogleTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    gtest_discover_tests(${TEST_EXECUTABLE_NAME})
endif()