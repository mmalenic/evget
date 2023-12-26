cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
include("${CMAKE_SOURCE_DIR}/utils.cmake")
project(util)

add_library(
        ${PROJECT_NAME}
        include/util/Util.h
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# add_subdirectory(src)

#if(BUILD_TESTING)
#    add_subdirectory(test)
#endif()