cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(evwatchsig)

add_library(${PROJECT_NAME} include/evwatchsig/EventWatcherSignal.h)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC evwatch)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
