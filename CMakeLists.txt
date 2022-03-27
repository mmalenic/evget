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

include(CheckIncludeFiles)
include(CheckCXXSymbolExists)
include(CTest)

# Checks the feature, returning the result true for the current feature.
function(feature_check)
    set(one_value_args NAME FEATURE)
    set(multi_value_args FEATURE_REQUIRES)
    cmake_parse_arguments(FEATURE_CHECK "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    get_property(CHECK GLOBAL PROPERTY ${FEATURE_CHECK_NAME})
    if (NOT "${CHECK}" STREQUAL "")
        message(STATUS "Skipping check for ${FEATURE_CHECK_NAME}, already checked.")
        return()
    endif()

    if (NOT DEFINED FEATURE_CHECK_FEATURE OR "${FEATURE_CHECK_FEATURE}" OR "${${FEATURE_CHECK_FEATURE}}")
        if (NOT DEFINED FEATURE_CHECK_FEATURE_REQUIRES OR FEATURE_CHECK_FEATURE_REQUIRES STREQUAL "")
            if (DEFINED FEATURE_CHECK_NAME)
                set_property(GLOBAL PROPERTY ${FEATURE_CHECK_NAME} TRUE)
            endif()
        else()
            check_include_files("${FEATURE_CHECK_FEATURE_REQUIRES}" FEATURE_CHECK_REQUIRES)
            set_property(GLOBAL PROPERTY ${FEATURE_CHECK_NAME} ${FEATURE_CHECK_REQUIRES})
        endif()
    endif()

    unset(FEATURE_CHECK_REQUIRES CACHE)
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
    set(one_value_args FEATURE FEATURE_CHECK)
    set(multi_value_args TARGET DEP FEATURE_REQUIRES COMPONENTS)
    cmake_parse_arguments(PROGRAM_DEPENDENCIES "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    set_feature_property(PROGRAM_DEPENDENCIES_FEATURE PROGRAM_DEPENDENCIES_FEATURE_CHECK PROGRAM_DEPENDENCIES_FEATURE_REQUIRES)
    if ((NOT DEFINED PROGRAM_DEPENDENCIES_FEATURE_CHECK OR GUARD) AND DEFINED PROGRAM_DEPENDENCIES_DEP)
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

        if (NOT ${DEPENDENCY_NAME}_FOUND)
            if (PROGRAM_DEPENDENCIES_NOT_REQUIRED AND (NOT DEFINED PROGRAM_DEPENDENCIES_COMPONENTS OR PROGRAM_DEPENDENCIES_FIND_ALL))
                find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION})
            elseif (PROGRAM_DEPENDENCIES_NOT_REQUIRED AND DEFINED PROGRAM_DEPENDENCIES_COMPONENTS)
                find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION} COMPONENTS ${COMPONENTS})
            elseif (NOT DEFINED PROGRAM_DEPENDENCIES_COMPONENTS OR PROGRAM_DEPENDENCIES_FIND_ALL)
                find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION} REQUIRED)
            else()
                find_package(${DEPENDENCY_NAME} ${DEPENDENCY_VERSION} REQUIRED COMPONENTS ${COMPONENTS})
            endif()
        endif()
        
        if (DEFINED DEPENDENCY_VERSION)
            message(STATUS "Found package ${DEPENDENCY_NAME} with version ${DEPENDENCY_VERSION}.")
        else()
            message(STATUS "Found package ${DEPENDENCY_NAME}.")
        endif()

        # Try to guess components
        if (NOT DEFINED PROGRAM_DEPENDENCIES_COMPONENTS)
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
    set(one_value_args TARGET FEATURE FEATURE_CHECK FEATURE_VISIBILITY INCLUDE_VISIBILITY)
    set(multi_value_args SRCS INCLUDE_DIRS FEATURE_REQUIRES)
    cmake_parse_arguments(ADD_PROGRAM "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    set_feature_property(ADD_PROGRAM_FEATURE ADD_PROGRAM_FEATURE_CHECK ADD_PROGRAM_FEATURE_REQUIRES)
    if (NOT DEFINED ADD_PROGRAM_FEATURE_CHECK OR GUARD)
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

                add_compile_definitions(GUARD ADD_PROGRAM_TARGET ADD_PROGRAM_FEATURE_CHECK ADD_PROGRAM_FEATURE_REQUIRES ADD_PROGRAM_FEATURE_VISIBILITY)
            elseif(ADD_PROGRAM_MAKE_EXECUTABLE)
                message(STATUS "Creating empty executable ${ADD_PROGRAM_TARGET}.")
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.cpp "")
                add_executable(${ADD_PROGRAM_TARGET} ${CMAKE_CURRENT_BINARY_DIR}/null.cpp)

                add_compile_definitions(GUARD ADD_PROGRAM_TARGET ADD_PROGRAM_FEATURE_CHECK ADD_PROGRAM_FEATURE_REQUIRES ADD_PROGRAM_FEATURE_VISIBILITY)
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

# Checks for the existence of symbols in files, and sets a compilation definition if present. By default all
# symbols must be present. If the ANY option is set, then only one of the symbols needs to be present.
function(find_symbols)
    set(options ANY)
    set(one_value_args TARGET VISIBILITY NAME)
    set(multi_value_args SYMBOLS FILES)
    cmake_parse_arguments(FIND_SYMBOLS "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    list(LENGTH FIND_SYMBOLS_SYMBOLS SYMBOLS_LENGTH)
    math(EXPR LOOP "${SYMBOLS_LENGTH} - 1")
    foreach(INDEX RANGE 0 ${LOOP})
        list(GET FIND_SYMBOLS_SYMBOLS ${INDEX} SYMBOL)
        check_cxx_symbol_exists(${SYMBOL} ${FIND_SYMBOLS_FILES} FIND_SYMBOLS_HAS_SYMBOL)

        if (FIND_SYMBOLS_ANY AND FIND_SYMBOLS_HAS_SYMBOL)
            message(STATUS "Adding compilation target ${FIND_SYMBOLS_NAME} for ${FIND_SYMBOLS_TARGET} with ${FIND_SYMBOLS_VISIBILITY} visibility.")
            target_compile_definitions(${FIND_SYMBOLS_TARGET} ${FIND_SYMBOLS_VISIBILITY} ${FIND_SYMBOLS_NAME})
            unset(FIND_SYMBOLS_HAS_SYMBOL CACHE)
            return()
        elseif(NOT FIND_SYMBOLS_ANY AND NOT FIND_SYMBOLS_HAS_SYMBOL)
            unset(FIND_SYMBOLS_HAS_SYMBOL CACHE)
            return()
        endif()

        unset(FIND_SYMBOLS_HAS_SYMBOL CACHE)
    endforeach()

    if (NOT FIND_SYMBOLS_ANY)
        message(STATUS "Adding compilation target ${FIND_SYMBOLS_NAME} for ${FIND_SYMBOLS_TARGET} with ${FIND_SYMBOLS_VISIBILITY} visibility.")
        target_compile_definitions(${FIND_SYMBOLS_TARGET} ${FIND_SYMBOLS_VISIBILITY} ${FIND_SYMBOLS_NAME})
    endif()
endfunction()

# Add compile definitons
macro(add_compile_definitions GUARD TARGET FEATURE_CHECK FEATURE_REQUIRES FEATURE_VISIBILITY)
    if (${GUARD} AND DEFINED ${FEATURE_CHECK} AND DEFINED ${FEATURE_REQUIRES})
        if (NOT DEFINED ${FEATURE_VISIBILITY})
            set(${FEATURE_VISIBILITY} PUBLIC)
        endif()

        get_target_property(DEFINITIONS ${${TARGET}} COMPILE_DEFINITIONS)
        if (NOT "${${FEATURE_CHECK}}" IN_LIST DEFINITIONS)
            message(STATUS "Adding compilation target ${${FEATURE_CHECK}} for ${${TARGET}} with ${${FEATURE_VISIBILITY}} visibility.")
            target_compile_definitions(${${TARGET}} ${${FEATURE_VISIBILITY}} ${${FEATURE_CHECK}})
        endif()
    endif()
endmacro()

# Set mode indicating how to add sources.
macro(set_mode MAKE_EXECUTABLE MAKE_LIBRARY)
    if (${MAKE_EXECUTABLE})
        set(MODE MAKE_EXECUTABLE)
    elseif(${MAKE_LIBRARY})
        set(MODE MAKE_LIBRARY)
    endif()
endmacro()

# Perform feature check and set property.
macro(set_feature_property FEATURE FEATURE_CHECK FEATURE_REQUIRES)
    if (DEFINED ${FEATURE_CHECK})
        feature_check(FEATURE ${${FEATURE}} FEATURE_REQUIRES ${${FEATURE_REQUIRES}} NAME ${${FEATURE_CHECK}})
        get_property(GUARD GLOBAL PROPERTY ${${FEATURE_CHECK}})
    endif()
endmacro()

# Sets a variable as a global property and retrieves it.
macro(set_and_get_variable NAME VALUE)
    set_property(GLOBAL PROPERTY ${NAME} ${VALUE})
    get_property(${NAME} GLOBAL PROPERTY ${NAME})
endmacro()

# Gets a global property.
macro(get_variable NAME)
    get_property(${NAME} GLOBAL PROPERTY ${NAME})
endmacro()

# Links the library to the test executable and discovers tests.
macro(link_testing TEST_EXECUTABLE_NAME LIBRARY_NAME)
    include(GoogleTest)
    enable_testing()

    program_dependencies(TARGET ${${TEST_EXECUTABLE_NAME}} DEP PRIVATE GTest COMPONENTS gtest gtest_main)
    program_dependencies(TARGET ${${TEST_EXECUTABLE_NAME}} DEP PUBLIC ${${LIBRARY_NAME}} ONLY_LINK)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    gtest_discover_tests(${${TEST_EXECUTABLE_NAME}})
endmacro()

# Optionally adds a subdirectory and links the dependencies.
macro(add_library_and_link TARGET DEPENDENCY DIRECTORY_NAME)
    if (NOT "${DIRECTORY_NAME}" STREQUAL "")
        add_subdirectory(${DIRECTORY_NAME})
    endif()

    get_variable(${DEPENDENCY})
    program_dependencies(TARGET ${${TARGET}} DEP PUBLIC ${${DEPENDENCY}} ONLY_LINK)
endmacro()

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
        VERIFY_SSL False
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

# Make executable.
add_program(
    TARGET ${CMAKE_PROJECT_NAME} MAKE_EXECUTABLE
    SRCS
    evgetcore/src/main.cpp
)

add_library_and_link(CMAKE_PROJECT_NAME CLIOPTION_LIBRARY_NAME clioption)
add_library_and_link(CMAKE_PROJECT_NAME LISTINPUTDEVICES_LIBRARY_NAME listinputdevices)
add_library_and_link(CMAKE_PROJECT_NAME EVGETCORE_LIBRARY_NAME evgetcore)
add_library_and_link(CMAKE_PROJECT_NAME EVGETX11_LIBRARY_NAME evgetx11)
