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

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/linux
    ${PROJECT_SOURCE_DIR}/include/win32_64
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/linux
    ${PROJECT_SOURCE_DIR}/src/win32_64
)

set(CMAKE_CXX_STANDARD 14)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71.0 COMPONENTS program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(input-event-recorder
        include/CommandLine.h
        include/linux/CommandLineLinux.h
        src/main.cpp
        src/CommandLine.cpp
        src/linux/CommandLineLinux.cpp
    )
    target_link_libraries(input-event-recorder ${Boost_LIBRARIES})
endif()
