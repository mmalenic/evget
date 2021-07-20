# Copyright (C) Marko Malenic 2021.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

    set(TESTS
        test/linux/CommandLineLinuxTest.cpp
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

