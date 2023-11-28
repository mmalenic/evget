cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(async)

add_library(
        ${PROJECT_NAME}
        include/async/coroutine/Scheduler.h
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(src)

program_dependencies(${PROJECT_NAME} spdlog VISIBILITY PUBLIC)
program_dependencies(${PROJECT_NAME}
        Boost
        VISIBILITY
        PUBLIC
        LINK_COMPONENTS
        headers
)

#if(BUILD_TESTING)
#    add_subdirectory(test)
#endif()