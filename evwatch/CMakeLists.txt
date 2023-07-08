cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(evwatch)

add_library(${PROJECT_NAME} include/evwatch/EventWatcher.h include/evwatch/EventWatcherException.h)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

# if (BUILD_TESTING) add_subdirectory(test) endif ()
