cmake_minimum_required(VERSION 3.19)
project(mouse_movement_recorder)

set(CMAKE_CXX_STANDARD 14)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71.0 COMPONENTS program_options)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(mouse_movement_recorder src/main.cpp src/CommandLine.cpp include/CommandLine.h)
    target_link_libraries(mouse_movement_recorder ${Boost_LIBRARIES})
endif()